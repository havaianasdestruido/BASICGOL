/*
 * workspace_desligado.c — prova de que a workspace falsa está DESLIGADA por
 * padrão. Compilado sem -DBG_USAR_WORKSPACE: o cabeçalho existe, mas não
 * declara nada (e BG_TEM_WORKSPACE não pode estar definido).
 */
#include <basicgol/workspace.h>

#ifdef BG_TEM_WORKSPACE
#error "BG_TEM_WORKSPACE não deveria existir sem BG_USAR_WORKSPACE"
#endif

int main(void)
{
    return 0;
}
