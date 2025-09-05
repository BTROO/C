#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifdef _WIN32
  #define CLEAR "cls"
#else
  #define CLEAR "clear"
#endif

#define MAX_PALAVRA 64
#define MAX_TENTATIVAS 6
#define MAX_PALAVRAS_ARQ 2048

/* ==================== Utilidades ==================== */

static void pausa_enter(void) {
    printf("\nPressione ENTER para continuar...");
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

static void limpar_tela(void) {
    system(CLEAR);
}

static void maiusculas(char *s) {
    for (; *s; ++s) *s = (char)toupper((unsigned char)*s);
}

static void tirar_quebra(char *s) {
    size_t n = strlen(s);
    if (n && (s[n-1] == '\n' || s[n-1] == '\r')) s[n-1] = '\0';
}

/* ==================== Forca (desenho) ==================== */

static void desenhar_forca(int erros) {
    const char *estagios[7] = {
        "  _______\n"
        " |/      |\n"
        " |       \n"
        " |       \n"
        " |       \n"
        " |       \n"
        "_|___    \n",

        "  _______\n"
        " |/      |\n"
        " |      ( )\n"
        " |       \n"
        " |       \n"
        " |       \n"
        "_|___    \n",

        "  _______\n"
        " |/      |\n"
        " |      ( )\n"
        " |       |\n"
        " |       \n"
        " |       \n"
        "_|___    \n",

        "  _______\n"
        " |/      |\n"
        " |      ( )\n"
        " |      /|\n"
        " |       \n"
        " |       \n"
        "_|___    \n",

        "  _______\n"
        " |/      |\n"
        " |      ( )\n"
        " |      /|\\\n"
        " |       \n"
        " |       \n"
        "_|___    \n",

        "  _______\n"
        " |/      |\n"
        " |      ( )\n"
        " |      /|\\\n"
        " |      / \n"
        " |       \n"
        "_|___    \n",

        "  _______\n"
        " |/      |\n"
        " |      ( )\n"
        " |      /|\\\n"
        " |      / \\\n"
        " |       \n"
        "_|___    \n"
    };
    if (erros < 0) erros = 0;
    if (erros > MAX_TENTATIVAS) erros = MAX_TENTATIVAS;
    printf("%s\n", estagios[erros]);
}

/* ==================== Banco de palavras ==================== */

static const char *PALAVRAS_PADRAO[] = {
    "COMPUTADOR","PROGRAMA","ALGORITMO","VARIAVEL","FUNCAO",
    "DESENVOLVEDOR","PORTFOLIO","GITHUB","JAVASCRIPT","REACT",
    "JAVA","LINGUAGEM","COMPILADOR","DEBUG","TERMINAL",
    "PROJETO","ARQUIVO","MEMORIA","PONTEIRO","ESTRUTURA",
    "MATRIZ","VETOR","CONDICAO","LACO","OPERATOR",
    "TECLADO","MOUSE","MONITOR","INTERNET","NAVEGADOR"
};
static const size_t N_PADRAO = sizeof(PALAVRAS_PADRAO)/sizeof(PALAVRAS_PADRAO[0]);

/* tenta carregar palavras de 'palavras.txt' (uma por linha, sem acentos de preferência) */
static size_t carregar_arquivo(char palavras[][MAX_PALAVRA], size_t max) {
    FILE *f = fopen("palavras.txt", "r");
    if (!f) return 0;

    size_t n = 0;
    char linha[256];
    while (n < max && fgets(linha, sizeof(linha), f)) {
        tirar_quebra(linha);
        if (linha[0] == '\0') continue;
        strncpy(palavras[n], linha, MAX_PALAVRA - 1);
        palavras[n][MAX_PALAVRA - 1] = '\0';
        maiusculas(palavras[n]);
        n++;
    }
    fclose(f);
    return n;
}

/* ==================== Jogo ==================== */

static void mostrar_estado(const char *mascara, int erros, const int usadas[26]) {
    desenhar_forca(erros);
    printf("Palavra: %s\n", mascara);

    printf("Letras usadas: ");
    int first = 1;
    for (int i = 0; i < 26; ++i) {
        if (usadas[i]) {
            if (!first) printf(", ");
            printf("%c", 'A' + i);
            first = 0;
        }
    }
    if (first) printf("(nenhuma)");
    printf("\nTentativas restantes: %d\n\n", MAX_TENTATIVAS - erros);
}

static int atualiza_mascara(const char *palavra, char *mascara, char letra) {
    int acertos = 0;
    for (size_t i = 0; palavra[i]; ++i) {
        if (palavra[i] == letra && mascara[i] == '_') {
            mascara[i] = letra;
            acertos++;
        }
    }
    return acertos;
}

static int palavra_completa(const char *mascara) {
    for (size_t i = 0; mascara[i]; ++i)
        if (mascara[i] == '_') return 0;
    return 1;
}

int main(void) {
    /* banco de palavras */
    char palavras_arq[MAX_PALAVRAS_ARQ][MAX_PALAVRA];
    size_t n_arq = carregar_arquivo(palavras_arq, MAX_PALAVRAS_ARQ);

    srand((unsigned)time(NULL));

    while (1) {
        limpar_tela();
        printf("===== JOGO DA FORCA =====\n");
        printf("1) Jogar\n");
        printf("2) Sair\n");
        printf("Escolha: ");
        int opc = 0;
        if (scanf("%d", &opc) != 1) return 0;
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}

        if (opc == 2) {
            printf("Até mais!\n");
            break;
        }
        if (opc != 1) continue;

        /* escolhe palavra */
        char palavra[MAX_PALAVRA];
        if (n_arq > 0) {
            size_t idx = (size_t)(rand() % n_arq);
            strncpy(palavra, palavras_arq[idx], MAX_PALAVRA);
        } else {
            size_t idx = (size_t)(rand() % N_PADRAO);
            strncpy(palavra, PALAVRAS_PADRAO[idx], MAX_PALAVRA);
        }
        palavra[MAX_PALAVRA-1] = '\0';
        maiusculas(palavra);

        /* cria máscara */
        char mascara[MAX_PALAVRA];
        size_t len = strlen(palavra);
        for (size_t i = 0; i < len; ++i) {
            mascara[i] = (palavra[i] >= 'A' && palavra[i] <= 'Z') ? '_' : palavra[i];
        }
        mascara[len] = '\0';

        int usadas[26] = {0};
        int erros = 0;

        /* loop do jogo */
        while (1) {
            limpar_tela();
            mostrar_estado(mascara, erros, usadas);

            if (erros >= MAX_TENTATIVAS) {
                printf("Você perdeu! A palavra era: %s\n", palavra);
                pausa_enter();
                break;
            }
            if (palavra_completa(mascara)) {
                printf("Parabéns! Você acertou: %s\n", palavra);
                pausa_enter();
                break;
            }

            printf("Digite uma letra (A-Z) ou tente a palavra inteira: ");
            char entrada[128];
            if (!fgets(entrada, sizeof(entrada), stdin)) return 0;
            tirar_quebra(entrada);
            maiusculas(entrada);

            if (strlen(entrada) == 0) continue;

            /* palpite de palavra inteira */
            if (strlen(entrada) > 1) {
                if (strcmp(entrada, palavra) == 0) {
                    strcpy(mascara, palavra);
                    limpar_tela();
                    mostrar_estado(mascara, erros, usadas);
                    printf("Acertou de primeira! A palavra é: %s\n", palavra);
                    pausa_enter();
                    break;
                } else {
                    printf("Palpite incorreto!\n");
                    erros++;
                    pausa_enter();
                    continue;
                }
            }

            /* palpite de única letra */
            char letra = entrada[0];
            if (letra < 'A' || letra > 'Z') {
                printf("Digite apenas letras de A-Z.\n");
                pausa_enter();
                continue;
            }

            int idx = letra - 'A';
            if (usadas[idx]) {
                printf("Você já tentou a letra '%c'. Tente outra.\n", letra);
                pausa_enter();
                continue;
            }
            usadas[idx] = 1;

            int acertou = atualiza_mascara(palavra, mascara, letra);
            if (!acertou) {
                printf("Letra '%c' não está na palavra.\n", letra);
                erros++;
            } else {
                printf("Boa! A letra '%c' aparece %d vez(es).\n", letra, acertou);
            }
            pausa_enter();
        }
    }

    return 0;
}
