#include <stdio.h>

#define WIDTH 5  // Larghezza della mappa
#define HEIGHT 5 // Altezza della mappa

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

// Mappa percorso obbligato
int map[HEIGHT][WIDTH] = {
    {0, 0, 0, 1, 0},
    {0, 1, 0, 1, 0},
    {0, 1, 0, 1, 0},
    {0, 1, 1, 0, 0},
    {0, 0, 0, 0, 1}};

Point start = {2, 2}; // Punto di partenza
Point goal = {4, 0};  // Punto di arrivo

/* // Mappa percorso chiuso
int map[HEIGHT][WIDTH] = {
    {1, 0, 0, 0, 0},
    {0, 1, 0, 0, 0},
    {0, 0, 1, 0, 0},
    {0, 0, 0, 1, 0},
    {0, 0, 0, 0, 1}};

Point start = {1, 4}; // Punto di partenza
Point goal = {3, 1};  // Punto di arrivo */

/* // Mappa doppio percorso
int map[HEIGHT][WIDTH] = {
    {0, 0, 0, 0, 0},
    {0, 1, 1, 1, 0},
    {0, 1, 1, 1, 0},
    {0, 1, 1, 1, 0},
    {0, 0, 0, 0, 0}};

Point start = {0, 4}; // Punto di partenza
Point goal = {2, 0};  // Punto di arrivo */

/* // Mappa errore
int map[HEIGHT][WIDTH] = {
    {0, 0, 0, 1, 0},
    {0, 1, 0, 1, 0},
    {0, 1, 0, 1, 0},
    {0, 1, 1, 0, 0},
    {0, 0, 0, 0, 1}};

Point start = {1, 1}; // Punto di partenza
Point goal = {0, 4};  // Punto di arrivo */

// Liste chiuse e aperte
int closedList[HEIGHT][WIDTH] = {0}; // Lista chiusa
Node openList[WIDTH * HEIGHT];       // Lista aperta
Node closedNodes[WIDTH * HEIGHT];    // Lista dei nodi chiusi
int openListSize = 0;
int closedListSize = 0;

Point path[WIDTH * HEIGHT]; // Array statico per il percorso
int pathLength = 0;         // Lunghezza del percorso

// Funzione per calcolare l'Heuristica (distanza euclidea)
int calculateHeuristic(Point start, Point goal)
{
    return (start.x - goal.x) * (start.x - goal.x) + (start.y - goal.y) * (start.y - goal.y);
}

// Funzione per aggiungere un nodo alla lista aperta
void addToOpenList(Node node)
{
    openList[openListSize++] = node;
}

// Funzione per aggiungere un nodo alla lista chiusa
void addToClosedList(Node node)
{
    closedNodes[closedListSize++] = node;
    closedList[node.point.y][node.point.x] = 1;
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

// Funzione per trovare un nodo nella lista chiusa
Node getNodeFromClosedList(Point p)
{
    for (int i = 0; i < closedListSize; i++)
    {
        if (isEqual(closedNodes[i].point, p))
        {
            return closedNodes[i];
        }
    }
    // Ritorna un nodo vuoto (non dovremmo mai arrivare qui se il percorso esiste)
    Node emptyNode = {{-1, -1}, 0, 0, 0, {-1, -1}};
    return emptyNode;
}

// Funzione per trovare il percorso utilizzando l'algoritmo A*
void aStar(Point start, Point goal)
{
    Node startNode = {start, 0, calculateHeuristic(start, goal), calculateHeuristic(start, goal), {-1, -1}};
    addToOpenList(startNode);

    while (openListSize > 0)
    {
        Node currentNode = getLowestFCostNode();
        addToClosedList(currentNode);

        if (isEqual(currentNode.point, goal))
        {
            printf("Percorso trovato!\n");
            Node pathNode = currentNode;

            // Ricostruisce il percorso risalendo i genitori
            while (pathNode.parent.x != -1 && pathNode.parent.y != -1)
            {
                path[pathLength++] = pathNode.point;
                pathNode = getNodeFromClosedList(pathNode.parent);
            }
            // Aggiunge il nodo di partenza al percorso
            path[pathLength++] = startNode.point;

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

                // Aggiunge il vicino alla lista aperta
                Node neighborNode = {neighbor, g_cost, h_cost, f_cost, currentNode.point};
                addToOpenList(neighborNode);
            }
        }
    }

    printf("Nessun percorso trovato.\n");
}

int main()
{
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
