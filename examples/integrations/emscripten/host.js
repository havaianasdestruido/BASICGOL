const exemplos = {
  "01_soma": {
    executar: (modulo) => `19 + 23 = ${modulo._bg_soma(19, 23)}`,
  },
  "02_fatorial": {
    executar: (modulo) => `10! = ${modulo._bg_fatorial(10)}`,
  },
  "03_media": {
    executar: (modulo) => `Média de 7.5, 8.5 e 9.0 = ${modulo._bg_media3(7.5, 8.5, 9.0).toFixed(2)}`,
  },
  "04_maior": {
    executar: (modulo) => `maior(-7, 12) = ${modulo._bg_maior(-7, 12)}`,
  },
  "05_primo": {
    executar: (modulo) => `97 é primo? ${modulo._bg_e_primo(97) === 1 ? "sim" : "não"}`,
  },
  "06_fahrenheit": {
    executar: (modulo) => `68 °F = ${modulo._bg_fahrenheit_para_celsius(68).toFixed(1)} °C`,
  },
  "07_limita": {
    executar: (modulo) => `limita(42, 0, 10) = ${modulo._bg_limita(42, 0, 10)}`,
  },
};

const seletor = document.querySelector("#exemplo");
const botao = document.querySelector("#executar");
const saida = document.querySelector("#resultado");

botao.addEventListener("click", async () => {
  const id = seletor.value;
  saida.textContent = "Carregando módulo WebAssembly…";
  try {
    const { default: criarModulo } = await import(`./build/${id}.js`);
    const modulo = await criarModulo();
    saida.textContent = exemplos[id].executar(modulo);
  } catch (erro) {
    console.error(erro);
    saida.textContent = `Não foi possível carregar ${id}. Rode ./build.sh e sirva esta pasta por HTTP.`;
  }
});
