#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

#define WIDTH 50  // Larghezza della mappa (aggiornata dinamicamente)
#define HEIGHT 50 // Altezza della mappa (aggiornata dinamicamente)

typedef struct
{
    int x, y;
} Point;

typedef struct
{
    Point point;
    int g_cost, h_cost, f_cost;
    Point parent;
} Node;

int map[HEIGHT][WIDTH];        // Mappa PGM caricata
int closedList[HEIGHT][WIDTH]; // Lista chiusa
Node openList[HEIGHT * WIDTH]; // Lista aperta
int openListSize = 0;

Point path[HEIGHT * WIDTH]; // Array per memorizzare il percorso
int pathLength = 0;         // Lunghezza del percorso

// Funzione per caricare la mappa PGM
void loadMap(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        printf("Errore nell'apertura del file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    char format[3];
    int max_value;
    fscanf(file, "%s", format);
    fscanf(file, "%d %d", &WIDTH, &HEIGHT);
    fscanf(file, "%d", &max_value);

    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            fscanf(file, "%d", &map[y][x]);
        }
    }

    fclose(file);
}

// Funzione per calcolare l'Heuristica (distanza euclidea)
int calculateHeuristic(Point start, Point goal)
{
    return (int)round(sqrt((start.x - goal.x) * (start.x - goal.x) + (start.y - goal.y) * (start.y - goal.y)));
}

// Funzione per aggiungere un nodo alla lista aperta
void addToOpenList(Node node)
{
    openList[openListSize++] = node;
}

// Funzione per trovare il nodo con il costo f più basso
Node getLowestFCostNode()
{
    int lowestIndex = 0;
    for (int i = 1; i < openListSize; i++)
    {
        if (openList[i].f_cost < openList[lowestIndex].f_cost)
        {
            lowestIndex = i;
        }
    }
    Node node = openList[lowestIndex];
    openListSize--;
    for (int i = lowestIndex; i < openListSize; i++)
    {
        openList[i] = openList[i + 1];
    }
    return node;
}

// Funzione per verificare se un punto è all'interno della mappa
int isValid(Point p)
{
    return p.x >= 0 && p.x < WIDTH && p.y >= 0 && p.y < HEIGHT;
}

// Funzione per verificare se un punto è un ostacolo
int isObstacle(Point p)
{
    return map[p.y][p.x] == 1; // 1 rappresenta un ostacolo
}

// Funzione per verificare se due punti sono uguali
int isEqual(Point a, Point b)
{
    return a.x == b.x && a.y == b.y;
}

// Funzione per trovare il percorso utilizzando l'algoritmo A*
void aStar(Point start, Point goal)
{
    Node startNode = {start, 0, calculateHeuristic(start, goal), calculateHeuristic(start, goal), {-1, -1}};
    addToOpenList(startNode);

    while (openListSize > 0)
    {
        Node currentNode = getLowestFCostNode();
        closedList[currentNode.point.y][currentNode.point.x] = 1;

        if (isEqual(currentNode.point, goal))
        {
            // Percorso trovato, traccia il percorso
            printf("Percorso trovato:\n");
            Node pathNode = currentNode;
            while (pathNode.parent.x != -1 && pathNode.parent.y != -1)
            {
                path[pathLength++] = pathNode.point; // Memorizza il punto nel percorso
                for (int i = 0; i < openListSize; i++)
                {
                    if (isEqual(openList[i].point, pathNode.parent))
                    {
                        pathNode = openList[i];
                        break;
                    }
                }
            }

            // Stampa il percorso dall'inizio alla fine
            for (int i = pathLength - 1; i >= 0; i--)
            {
                printf("(%d, %d)\n", path[i].x, path[i].y);
            }
            return;
        }

        // Genera i vicini
        Point neighbors[4] = {
            {currentNode.point.x + 1, currentNode.point.y},
            {currentNode.point.x - 1, currentNode.point.y},
            {currentNode.point.x, currentNode.point.y + 1},
            {currentNode.point.x, currentNode.point.y - 1}};

        for (int i = 0; i < 4; i++)
        {
            Point neighbor = neighbors[i];
            if (isValid(neighbor) && !isObstacle(neighbor) && !closedList[neighbor.y][neighbor.x])
            {
                int g_cost = currentNode.g_cost + 1;
                int h_cost = calculateHeuristic(neighbor, goal);
                int f_cost = g_cost + h_cost;

                Node neighborNode = {neighbor, g_cost, h_cost, f_cost, currentNode.point};
                addToOpenList(neighborNode);
            }
        }
    }

    printf("Nessun percorso trovato.\n");
}

int main()
{
    Point start, goal;

    printf("Inserisci le coordinate del punto di partenza (x y): ");
    scanf("%d %d", &start.x, &start.y);

    printf("Inserisci le coordinate del punto di arrivo (x y): ");
    scanf("%d %d", &goal.x, &goal.y);

    loadMap("/mnt/c/Users/andre/OneDrive/Desktop/mappa1.pgm");

    if (!isValid(start) || isObstacle(start))
    {
        printf("Il punto di partenza non è valido o è un ostacolo.\n");
        return 1;
    }

    if (!isValid(goal) || isObstacle(goal))
    {
        printf("Il punto di arrivo non è valido o è un ostacolo.\n");
        return 1;
    }

    aStar(start, goal);

    return 0;
}
