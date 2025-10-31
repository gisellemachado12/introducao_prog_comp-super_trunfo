#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/* ================================
   Modelo de dados
   ================================ */
typedef struct {
    char estado;                 // A-H
    char codigo[5];              // ex: A01 (4 + '\0')
    char cidade[50];
    unsigned long populacao;
    double area_km2;
    double pib_bilhoes;          // PIB em bilhões
    int pontos_turisticos;
    double densidade;            // hab/km² (calculada)
    double pib_per_capita;       // (calculado)
} FichaCidade;

/* ================================
   Atributos disponíveis
   ================================ */
enum Atributo {
    ATR_POPULACAO = 1,
    ATR_AREA      = 2,
    ATR_PIB       = 3,
    ATR_TURISMO   = 4,
    ATR_DENSIDADE = 5, // menor densidade é melhor -> invertida na pontuação
    ATR_PIB_PC    = 6
};

static const char* nome_atributo(int atr) {
    switch (atr) {
        case ATR_POPULACAO: return "População";
        case ATR_AREA:      return "Área";
        case ATR_PIB:       return "PIB";
        case ATR_TURISMO:   return "Pontos Turísticos";
        case ATR_DENSIDADE: return "Densidade Demográfica";
        case ATR_PIB_PC:    return "PIB per Capita";
        default:            return "Desconhecido";
    }
}

/* ================================
   Utilitários de I/O
   ================================ */
static void limpar_buffer_stdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { /* descarta */ }
}

static void ler_texto(const char* prompt, char* buf, size_t tam) {
    printf("%s", prompt);
    if (fgets(buf, (int)tam, stdin)) {
        buf[strcspn(buf, "\n")] = '\0';
    } else {
        // se falhar, garante string vazia
        if (tam > 0) buf[0] = '\0';
        clearerr(stdin);
    }
}

static void ler_char(const char* prompt, char* out) {
    printf("%s", prompt);
    if (scanf(" %c", out) != 1) {
        *out = 'A';
    }
    limpar_buffer_stdin();
}

static void ler_ulong(const char* prompt, unsigned long* out) {
    printf("%s", prompt);
    while (scanf("%lu", out) != 1) {
        limpar_buffer_stdin();
        printf("Valor inválido. Tente novamente: ");
    }
    limpar_buffer_stdin();
}

static void ler_double(const char* prompt, double* out) {
    printf("%s", prompt);
    while (scanf("%lf", out) != 1) {
        limpar_buffer_stdin();
        printf("Valor inválido. Tente novamente: ");
    }
    limpar_buffer_stdin();
}

static void ler_int(const char* prompt, int* out) {
    printf("%s", prompt);
    while (scanf("%d", out) != 1) {
        limpar_buffer_stdin();
        printf("Valor inválido. Tente novamente: ");
    }
    limpar_buffer_stdin();
}

/* ================================
   Cálculos derivados
   ================================ */
static void calcular_metricas(FichaCidade* f) {
    if (!f) return;

    if (f->area_km2 > 0.0) {
        f->densidade = (double)f->populacao / f->area_km2;
    } else {
        f->densidade = 0.0; // evita divisão por zero (trataremos na pontuação)
    }

    if (f->populacao > 0UL) {
        // PIB vem em bilhões → converte para unidade monetária absoluta
        f->pib_per_capita = (f->pib_bilhoes * 1e9) / (double)f->populacao;
    } else {
        f->pib_per_capita = 0.0;
    }
}

/* ================================
   Leitura de uma ficha/cidade
   ================================ */
static void ler_ficha(FichaCidade* f, const char* titulo) {
    if (!f) return;

    printf("\n=== %s ===\n", titulo);

    ler_char("Estado (A-H): ", &f->estado);

    printf("Código da Carta (ex: A01): ");
    if (scanf("%4s", f->codigo) != 1) {
        strcpy(f->codigo, "A01");
    }
    limpar_buffer_stdin();

    ler_texto("Nome da Cidade: ", f->cidade, sizeof(f->cidade));
    ler_ulong("População: ", &f->populacao);
    ler_double("Área (km²): ", &f->area_km2);
    ler_double("PIB (em bilhões): ", &f->pib_bilhoes);
    ler_int("Número de Pontos Turísticos: ", &f->pontos_turisticos);

    calcular_metricas(f);
}

/* ================================
   Obter valor “base” do atributo
   ================================ */
static double valor_atributo_base(const FichaCidade* f, int atr) {
    switch (atr) {
        case ATR_POPULACAO: return (double)f->populacao;
        case ATR_AREA:      return f->area_km2;
        case ATR_PIB:       return f->pib_bilhoes;
        case ATR_TURISMO:   return (double)f->pontos_turisticos;
        case ATR_DENSIDADE: return f->densidade;
        case ATR_PIB_PC:    return f->pib_per_capita;
        default:            return 0.0;
    }
}

/* ================================
   Pontuação (regras por atributo)
   - Para DENSIDADE: menor é melhor → usamos 1/x
   - Para os demais: maior é melhor → usamos x
   ================================ */
static double pontuar_atributo(const FichaCidade* f, int atr) {
    double v = valor_atributo_base(f, atr);
    if (atr == ATR_DENSIDADE) {
        return (v > 0.0) ? (1.0 / v) : 0.0;
    }
    return v;
}

/* ================================
   Menu de atributos e leitura
   ================================ */
static void mostrar_menu(void) {
    puts("\nAtributos disponíveis:");
    puts("1 - População");
    puts("2 - Área");
    puts("3 - PIB");
    puts("4 - Pontos Turísticos");
    puts("5 - Densidade Demográfica (menor é melhor)");
    puts("6 - PIB per Capita");
}

static int ler_atributo_distinto(const char* prompt, int diferente_de) {
    int a = 0;
    while (1) {
        ler_int(prompt, &a);
        if (a < 1 || a > 6) {
            puts("Atributo inválido. Escolha entre 1 e 6.");
            continue;
        }
        if (a == diferente_de) {
            puts("Atributo já escolhido. Selecione outro.");
            continue;
        }
        return a;
    }
}

/* ================================
   Execução principal
   ================================ */
int main(void) {
    FichaCidade c1 = {0}, c2 = {0};
    int atr1 = 0, atr2 = 0;

    // Cadastro
    ler_ficha(&c1, "Cadastro da Carta 1");
    ler_ficha(&c2, "Cadastro da Carta 2");

    // Escolha dos atributos
    mostrar_menu();
    atr1 = ler_atributo_distinto("Escolha o primeiro atributo para comparação: ", 0);
    atr2 = ler_atributo_distinto("Escolha o segundo atributo (diferente do primeiro): ", atr1);

    // Mostrar comparação
    double c1_v1_base = valor_atributo_base(&c1, atr1);
    double c2_v1_base = valor_atributo_base(&c2, atr1);
    double c1_v2_base = valor_atributo_base(&c1, atr2);
    double c2_v2_base = valor_atributo_base(&c2, atr2);

    printf("\nComparando %s e %s\n", c1.cidade, c2.cidade);
    printf("Atributo 1: %s\n", nome_atributo(atr1));
    printf("  %s: %.2f\n", c1.cidade, c1_v1_base);
    printf("  %s: %.2f\n", c2.cidade, c2_v1_base);

    printf("Atributo 2: %s\n", nome_atributo(atr2));
    printf("  %s: %.2f\n", c1.cidade, c1_v2_base);
    printf("  %s: %.2f\n", c2.cidade, c2_v2_base);

    // Pontuação (aplica regra “menor densidade vence” automaticamente)
    double c1_score = pontuar_atributo(&c1, atr1) + pontuar_atributo(&c1, atr2);
    double c2_score = pontuar_atributo(&c2, atr1) + pontuar_atributo(&c2, atr2);

    printf("\nResultado Final (após regras por atributo):\n");
    printf("%s: %.4f\n", c1.cidade, c1_score);
    printf("%s: %.4f\n", c2.cidade, c2_score);

    printf("Vencedor: %s\n",
           (c1_score > c2_score) ? c1.cidade :
           (c2_score > c1_score) ? c2.cidade : "Empate!");

    return 0;
}
