"""Testes do gerador: descoberta, evidência, portabilidade e determinismo."""
from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from tools.documentacao.analise import collect
from tools.documentacao.render import Renderer, protect
from tools.gerar_documentacao import check_outputs, clean_managed_output, tree_digest

ROOT = Path(__file__).resolve().parents[2]


class GeradorTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        """Extrai o repositório real uma vez para os testes de integração."""
        cls.inv = collect(ROOT)

    def test_funcoes_publicas_tem_corpo_e_prototipo(self) -> None:
        """Funções bg_* públicas são ligadas à declaração do cabeçalho."""
        for name in ("bg_leia_linha", "bg_copia_texto", "bg_divide_i", "bg_sorteia_ate"):
            symbols = self.inv.by_name[name]
            self.assertEqual(len(symbols), 1, name)
            self.assertTrue(symbols[0].body, name)
            self.assertTrue(symbols[0].prototypes, name)

    def test_apelidos_e_expansao(self) -> None:
        """SE e BG_SE são reunidos sem perder a expansão literal."""
        se = self.inv.by_name["BG_SE"][0]
        self.assertIn("SE", se.aliases)
        self.assertEqual(se.body.strip(), "if (")

    def test_guards_de_compilador_preservados(self) -> None:
        """A análise não escolhe um alvo e apaga os demais ramos."""
        soma = self.inv.by_name["bg_soma_segura"][0]
        conditions = " ".join(c["condition"] for c in soma.conditions)
        self.assertIn("BG_USAR_EXTENSOES", conditions)
        self.assertTrue("__GNUC__" in conditions or "__clang__" in conditions)
        formata = self.inv.by_name["bg_formata_texto"][0]
        source = self.inv.sources[formata.path]
        window = "\n".join(source.lines[formata.line - 8:formata.end + 8])
        self.assertIn("__clang__", window)
        self.assertIn("pragma", window)

    def test_referencias_de_teste_tem_linha_real(self) -> None:
        """Uma chamada de teste aponta para arquivo e linha existentes."""
        refs = [r for r in self.inv.references if r.symbol == "bg_leia_linha"
                and r.path == "tests/test_biblioteca.c"]
        self.assertTrue(refs)
        source = self.inv.sources["tests/test_biblioteca.c"]
        for ref in refs:
            self.assertIn("bg_leia_linha", source.lines[ref.line - 1])

    def test_casos_multiarquivo_e_marcadores(self) -> None:
        """Descoberta inclui todos os .c e marcadores, não só programa.c."""
        cases = {c["name"]: c for c in self.inv.cases}
        self.assertGreater(len(cases["m01_modulos"]["programs"]), 1)
        self.assertFalse(cases["c15_para_acucar"]["m0"])
        self.assertIn("sem_m0", cases["c15_para_acucar"]["markers"])
        self.assertTrue(cases["m01_modulos"]["runner_registered"])

    def test_modos_fatais_e_stdin(self) -> None:
        """strcmp do harness é ligado ao ramo e às primitivas chamadas."""
        modes = {m["mode"]: m for m in self.inv.modes}
        self.assertIn("bg_divide_i", modes["divide-zero"]["calls"])
        self.assertIn("testa_stdin_compativel", modes["--stdin"]["calls"])

    def test_python_e_shell_descobertos(self) -> None:
        """Funções além do C entram no inventário interno."""
        self.assertIn("caso_execucao", self.inv.by_name)
        self.assertIn("muta", self.inv.by_name)
        self.assertEqual(self.inv.by_name["caso_execucao"][0].language, "shell")
        self.assertEqual(self.inv.by_name["muta"][0].language, "python")

    def test_escrita_mdx_nao_quebra_codigo(self) -> None:
        """Chaves e ângulos em prosa são escapados; código inline permanece."""
        text = "use <basicgol.h> e {x}, mas `SE x { y; }` fica intacto"
        escaped = protect(text)
        self.assertIn("&lt;basicgol.h>", escaped)
        self.assertIn("&#123;x&#125;", escaped)
        self.assertIn("`SE x { y; }`", escaped)

    def test_saida_deterministica_e_check(self) -> None:
        """Dois builds dos mesmos insumos têm hashes iguais; --check detecta drift."""
        import json
        contracts = json.loads((ROOT / "tools/documentacao/contratos.json").read_text(encoding="utf-8"))
        with tempfile.TemporaryDirectory() as tmp:
            base = Path(tmp)
            for folder in ("a", "b"):
                renderer = Renderer(self.inv, contracts, "https://github.com/havaianasdestruido/BASICGOL", "test-commit")
                renderer.write(base / folder / "docs")
            self.assertEqual(tree_digest(base / "a"), tree_digest(base / "b"))
            self.assertEqual(check_outputs(base / "a", base / "b" / "docs", base / "b" / "static" / "api"), [])
            (base / "b" / "docs" / "stale.mdx").write_text("stale", encoding="utf-8")
            self.assertTrue(any("obsoleto" in p for p in check_outputs(base / "a", base / "b" / "docs", base / "b" / "static" / "api")))

    def test_nao_apaga_saida_nao_gerenciada(self) -> None:
        """Limpeza recusa diretório sem marcador de geração."""
        with tempfile.TemporaryDirectory() as tmp:
            docs = Path(tmp) / "docs"
            docs.mkdir()
            (docs / "manual.md").write_text("não apagar", encoding="utf-8")
            with self.assertRaises(RuntimeError):
                clean_managed_output(docs, Path(tmp) / "api")
            self.assertTrue((docs / "manual.md").exists())

    def test_funcao_e_caso_novos_aparecem_sem_lista_manual(self) -> None:
        """Um repositório mínimo prova a descoberta automática de adições futuras."""
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "include").mkdir()
            (root / "src" / "biblioteca").mkdir(parents=True)
            (root / "tests" / "casos" / "20_nova" / "n01").mkdir(parents=True)
            (root / "include" / "mini.h").write_text(
                "#define BG_NOVO(x) ((x) + 1)\n#define NOVO BG_NOVO\nint novo(int x);\n", encoding="utf-8")
            (root / "src" / "biblioteca" / "mini.c").write_text(
                "int novo(int x) { return x + 1; }\n", encoding="utf-8")
            (root / "tests" / "run.sh").write_text("for suite in 20_nova; do echo $suite; done\n", encoding="utf-8")
            case = root / "tests" / "casos" / "20_nova" / "n01"
            (case / "programa.c").write_text("int main(void) { return NOVO(0); }\n", encoding="utf-8")
            (case / "esperado.txt").write_text("1\n", encoding="utf-8")
            inv = collect(root)
            self.assertIn("novo", inv.by_name)
            self.assertIn("BG_NOVO", inv.by_name)
            self.assertIn("NOVO", inv.by_name["BG_NOVO"][0].aliases)
            self.assertEqual(inv.cases[0]["suite"], "20_nova")
            self.assertTrue(inv.cases[0]["runner_registered"])


if __name__ == "__main__":
    unittest.main()
