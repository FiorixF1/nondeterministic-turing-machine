#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FINAL 2

// ---------- STRUTTURE DATI ---------- //

enum MovimentoTestina {LEFT, STANDING, RIGHT};

typedef struct {
    int stato;
    char *memoria;
    int posizione;
    int quantita_memoria;
} Configurazione;

typedef struct ListaConfigurazioni {
    Configurazione config;
    struct ListaConfigurazioni *next;
    struct ListaConfigurazioni *prev;
} ListaConfigurazioni;

typedef struct {
    int stato_iniziale;
    char carattere_input;
    char carattere_output;
    int stato_finale;
    enum MovimentoTestina movimento;
} Transizione;

typedef struct ListaTransizioni {
    Transizione trans;
    struct ListaTransizioni *next;
} ListaTransizioni;

// !--------- STRUTTURE DATI ---------! //

// -------- VARIABILI GLOBALI --------- //

unsigned int MAX_MOVES;
char *stati;
int numero_stati = 1;
ListaTransizioni ***transizioni;

// !------- VARIABILI GLOBALI --------! //

// ------------- FUNZIONI ------------- //

void insertTransition(int stato_iniziale, char carattere_input, char carattere_output, int stato_finale, enum MovimentoTestina movimento) {
    if (transizioni == NULL) {
        transizioni = malloc(numero_stati*sizeof(ListaTransizioni **));
        for (int i = 0; i < numero_stati; ++i) {
            transizioni[i] = calloc(74, sizeof(ListaTransizioni *));
        }
    }
        
    ListaTransizioni *nuovo = malloc(sizeof(ListaTransizioni));
    nuovo->trans.stato_iniziale = stato_iniziale;
    nuovo->trans.carattere_input = carattere_input;
    nuovo->trans.carattere_output = carattere_output;
    nuovo->trans.stato_finale = stato_finale;
    nuovo->trans.movimento = movimento;

    if (stato_iniziale >= numero_stati || stato_finale >= numero_stati) {
        int new = stato_iniziale+1;
        if (stato_finale > stato_iniziale) new = stato_finale+1;

        transizioni = realloc(transizioni, new*sizeof(ListaTransizioni **));
        for (int i = numero_stati; i < new; ++i) {
            transizioni[i] = calloc(74, sizeof(ListaTransizioni *));
        }
        numero_stati = new;
    }
    
    int i = stato_iniziale;
    int j = carattere_input-48;
    nuovo->next = transizioni[i][j];
    transizioni[i][j] = nuovo;
}

void insertAcceptingState(int stato) {
    stati[stato] = FINAL;
}

ListaConfigurazioni *initConfig(char *input) {
    ListaConfigurazioni *first = malloc(sizeof(ListaConfigurazioni));
    first->config.stato = 0;
    first->config.memoria = input;
    first->config.posizione = 0;
    first->config.quantita_memoria = strlen(input);
    first->config.memoria = realloc(first->config.memoria, strlen(first->config.memoria));  // elimina il terminatore
    first->next = NULL;
    first->prev = NULL;
    return first;
}

ListaConfigurazioni *insertConfiguration(ListaConfigurazioni *lista, int stato, char *memoria, int posizione, int quantita_memoria) {
    ListaConfigurazioni *nuovo = malloc(sizeof(ListaConfigurazioni));
    nuovo->config.stato = stato;
    nuovo->config.memoria = malloc(sizeof(char)*quantita_memoria);
    memcpy(nuovo->config.memoria, memoria, quantita_memoria);
    nuovo->config.posizione = posizione;
    nuovo->config.quantita_memoria = quantita_memoria;
    nuovo->next = lista;
    nuovo->prev = NULL;
    if (lista != NULL) lista->prev = nuovo;
    return nuovo;
}

ListaConfigurazioni *deleteConfiguration(ListaConfigurazioni *lista, ListaConfigurazioni *config) {
    if (lista == NULL) {
        return NULL;
    }

    if (config->prev == NULL) {
        lista = config->next;
        if (lista != NULL) lista->prev = NULL;
        free(config->config.memoria);
        free(config);
        return lista;
    } else {
        config->prev->next = config->next;
        if (config->next != NULL) config->next->prev = config->prev;
        free(config->config.memoria);
        free(config);
        return lista;
    }
}

void deleteConfigs(ListaConfigurazioni *lista) {
    while (lista != NULL) {
        ListaConfigurazioni *corrente = lista;
        lista = lista->next;
        free(corrente->config.memoria);
        free(corrente);
    }
}

int isAccepting(int stato) {
    if (stati[stato] == FINAL) return 1;
    else return 0;
}

ListaTransizioni *getActions(Configurazione *config, ListaTransizioni ***lista_tr) {
    int i = config->stato;
    int j = config->memoria[config->posizione]-48;
    return lista_tr[i][j];
}

void executeMovement(Configurazione *config, enum MovimentoTestina movement) {
    switch (movement) {
        case LEFT:
            if (config->posizione > 0) {
                --config->posizione;
            } else {
                int new_quantity = config->quantita_memoria << 1;
                int old_position = config->posizione;
                config->memoria = realloc(config->memoria, new_quantity);
                config->quantita_memoria = new_quantity;

                memcpy(config->memoria + (new_quantity >> 1), config->memoria, new_quantity >> 1);
                memset(config->memoria, '_', new_quantity >> 1);

                config->posizione = old_position + new_quantity >> 1;
                --config->posizione;
            }
            break;
        case STANDING:
            return;
        case RIGHT:
            if (config->posizione < config->quantita_memoria-1) {
                ++config->posizione;
            } else {
                int new_quantity = config->quantita_memoria << 1;
                config->memoria = realloc(config->memoria, new_quantity);
                config->quantita_memoria = new_quantity;
                memset(config->memoria + (new_quantity >> 1), '_', new_quantity >> 1);
                ++config->posizione;
            }
            break;
    }
}

int executeTransition(Configurazione *config, Transizione *trans, int DETERMINISTIC) {

    // 0 a a 0 S
    if (trans->stato_iniziale == trans->stato_finale
        && trans->carattere_input == trans->carattere_output
        && trans->movimento == STANDING) {
        return 1;
    }

    if (DETERMINISTIC) {
        // 0 _ * 0 L
        if (trans->stato_iniziale == trans->stato_finale
            && trans->carattere_input == '_'
            && trans->movimento == LEFT
            && config->posizione == 0) {
            return 1;
        }

        // 0 _ * 0 R
        if (trans->stato_iniziale == trans->stato_finale
            && trans->carattere_input == '_'
            && trans->movimento == RIGHT
            && config->posizione == config->quantita_memoria-1) {
            return 1;
        }
    }

    config->stato = trans->stato_finale;
    config->memoria[config->posizione] = trans->carattere_output;
    executeMovement(config, trans->movimento);
    return 0;
}

void execute(char *input) {
    unsigned int contatore = 0;

    // crea configurazione iniziale
    ListaConfigurazioni *configs = initConfig(input);

    int UNDEFINED = 0;
    while (contatore <= MAX_MOVES) {
        // caso nessuna configurazione esistente -> esci dal while
        if (configs == NULL) break;

        // per ogni configurazione
        ListaConfigurazioni *cursore_c = configs;
        while (cursore_c != NULL) {
            // cerca transizioni ammissibili
            ListaTransizioni *azioni = getActions(&cursore_c->config, transizioni);
            if (azioni == NULL) {
                ListaConfigurazioni *temp = cursore_c->next;
                configs = deleteConfiguration(configs, cursore_c);
                cursore_c = temp;
                continue;
            } else {
                int DETERMINISTIC = 0;
                if (azioni->next == NULL) DETERMINISTIC = 1;

                ListaTransizioni *cursore_a = azioni;

                // per ogni configurazione (tranne l'ultima)
                while (cursore_a->next != NULL) {
                    // aggiungi nuova configurazione in testa
                    configs = insertConfiguration(configs, cursore_c->config.stato, cursore_c->config.memoria, cursore_c->config.posizione, cursore_c->config.quantita_memoria);

                    // mh...
                    int LOOP = executeTransition(&configs->config, &cursore_a->trans, DETERMINISTIC);
                    if (LOOP) {
                        UNDEFINED = 1;
                        configs = deleteConfiguration(configs, configs);
                        cursore_a = cursore_a->next;
                        continue;
                    } else if (isAccepting(configs->config.stato)) {
                        printf("1\n");
                        deleteConfigs(configs);
                        return;
                    }

                    cursore_a = cursore_a->next;
                }
                // l'ultima transizione si esegue in-place
                int LOOP = executeTransition(&cursore_c->config, &cursore_a->trans, DETERMINISTIC);
                if (LOOP) {
                    UNDEFINED = 1;
                    ListaConfigurazioni *temp = cursore_c->next;
                    configs = deleteConfiguration(configs, cursore_c);
                    cursore_c = temp;
                    continue;
                } else if (isAccepting(cursore_c->config.stato)) {
                    printf("1\n");
                    deleteConfigs(configs);
                    return;
                }
            }
            cursore_c = cursore_c->next;
        }

        ++contatore;
    }
    if (contatore > MAX_MOVES || UNDEFINED) {
        printf("U\n");
    } else {
        printf("0\n");
    }
    deleteConfigs(configs);
}

// !------------ FUNZIONI ------------! //


int main() {
    int numero_stati = 0;

    // tr
    char tr[5];

    scanf("%s\n", tr);

    // lettura transizioni
    char input_transizioni[32];
    while (1) {
        fgets(input_transizioni, 32, stdin);

        if (input_transizioni[strlen(input_transizioni)-2] == '\r') {
            input_transizioni[strlen(input_transizioni)-2] = '\0';
        } else {
            input_transizioni[strlen(input_transizioni)-1] = '\0';
        }

        char acc[6];
        sscanf(input_transizioni, "%s", acc);

        if (strcmp(acc, "acc") == 0) {
            break;
        }

        int stato_iniziale, stato_finale;
        char carattere_input, carattere_output, movimento;
        enum MovimentoTestina enumMovimento;

        sscanf(input_transizioni, "%d %c %c %c %d", &stato_iniziale, &carattere_input, &carattere_output, &movimento, &stato_finale);

        if (movimento == 'R') {
            enumMovimento = RIGHT;
        } else if (movimento == 'L') {
            enumMovimento = LEFT;
        } else {
            enumMovimento = STANDING;
        }

        if (stato_iniziale+1 > numero_stati) numero_stati = stato_iniziale+1;
        if (stato_finale+1 > numero_stati) numero_stati = stato_finale+1;

        insertTransition(stato_iniziale, carattere_input, carattere_output, stato_finale, enumMovimento);
    }

    stati = calloc(numero_stati, sizeof(char));

    // stati accettazione
    char input_accettazione[32];
    while (1) {
        fgets(input_accettazione, 32, stdin);
        if (input_accettazione[strlen(input_accettazione)-2] == '\r') {
            input_accettazione[strlen(input_accettazione)-2] = '\0';
        } else {
            input_accettazione[strlen(input_accettazione)-1] = '\0';
        }

        char max[6];
        sscanf(input_accettazione, "%s", max);
        if (strcmp(max, "max") == 0) break;

        int stato_accettazione;
        sscanf(input_accettazione, "%d", &stato_accettazione);

        insertAcceptingState(stato_accettazione);
    }

    // numero massimo di passi
    char input_max[32];
    fgets(input_max, 32, stdin);
    if (input_max[strlen(input_max)-2] == '\r') {
        input_max[strlen(input_max)-2] = '\0';
    } else {
        input_max[strlen(input_max)-1] = '\0';
    }
    sscanf(input_max, "%d", &MAX_MOVES);

    // run
    char run[6];
    scanf("%s\n", run);

    // lettura stringhe ed esecuzione
    while (1) {
        char *input_stringa;
        int code = scanf("%ms", &input_stringa);

        if (code == EOF) break;

        long length = strlen(input_stringa);
        if (length == 0) continue;
        if (input_stringa[length-1] == '\r') {
            input_stringa[length-1] = '\0';
        }

        execute(input_stringa);
    }

    return 0;
}