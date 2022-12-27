//
// AED, November 2022 (Tomás Oliveira e Silva)
//
// Second practical assignement (Word Ladder)
//
// Place your student numbers and names here
//   N.Mec. 107348  Name:  Pedro Ramos
//   N.Mec. 105925  Name : Rafael Kauati
//
// Do as much as you can
//   1) MANDATORY: complete the hash table code
//      *) hash_table_create
//      *) hash_table_grow
//      *) hash_table_free
//      *) find_word
//      +) add code to get some statistical data about the hash table
//   2) HIGHLY RECOMMENDED: build the graph (including union-find data) -- use the similar_words function...
//      *) find_representative
//      *) add_edge
//   3) RECOMMENDED: implement breadth-first search in the graph
//      *) breadh_first_search
//   4) RECOMMENDED: list all words belonginh to a connected component
//      *) breadh_first_search
//      *) list_connected_component
//   5) RECOMMENDED: find the shortest path between to words
//      *) breadh_first_search
//      *) path_finder
//      *) test the smallest path from bem to mal
//         [ 0] bem
//         [ 1] tem
//         [ 2] teu
//         [ 3] meu
//         [ 4] mau
//         [ 5] mal
//      *) find other interesting word ladders
//   6) OPTIONAL: compute the diameter of a connected component and list the longest word chain
//      *) breadh_first_search
//      *) connected_component_diameter
//   7) OPTIONAL: print some statistics about the graph
//      *) graph_info
//   8) OPTIONAL: test for memory leaks
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//
// static configuration
//

#define _max_word_size_ 32



int totalWords = 0;
int totalColisions = 0;
int mostColHashNode = 0;
int mostColisions = 0;
int totalGrows = 0;
int numUsedHashNodes = 0;

int totalEdges = 0;
int numConnectedComponents = 0;
int numSeperatedComponents = 0;
int largestComponent = 0;


//
// data structures (SUGGESTION --- you may do it in a different way)
//

typedef struct adjacency_node_s adjacency_node_t;
typedef struct hash_table_node_s hash_table_node_t;
typedef struct hash_table_s hash_table_t;

struct adjacency_node_s
{
  adjacency_node_t *next;            // link to the next adjacency list node
  hash_table_node_t *vertex;         // the other vertex
};

struct hash_table_node_s
{
  // the hash table data
  char word[_max_word_size_];        // the word
  hash_table_node_t *next;           // next hash table linked list node
  // the vertex data
  adjacency_node_t *head;            // head of the linked list of adjancency edges
  int visited;                       // visited status (while not in use, keep it at 0)
  hash_table_node_t *previous;       // breadth-first search parent
  // the union find data
  hash_table_node_t *representative; // the representative of the connected component this vertex belongs to
  int number_of_vertices;            // number of vertices of the conected component (only correct for the representative of each connected component)
  int number_of_edges;               // number of edges of the conected component (only correct for the representative of each connected component)
};

struct hash_table_s
{
  unsigned int hash_table_size;      // the size of the hash table array
  unsigned int number_of_entries;    // the number of entries in the hash table
  unsigned int number_of_edges;      // number of edges (for information purposes only)
  hash_table_node_t **heads;         // the heads of the linked lists
};

//
// allocation and deallocation of linked list nodes (done)
//

static adjacency_node_t *allocate_adjacency_node(void)
{
  adjacency_node_t *node;

  node = (adjacency_node_t *)malloc(sizeof(adjacency_node_t));
  if(node == NULL)
  {
    fprintf(stderr,"allocate_adjacency_node: out of memory\n");
    exit(1);
  }
  return node;
}

static void free_adjacency_node(adjacency_node_t *node)
{
  free(node);
}

static hash_table_node_t *allocate_hash_table_node(void)
{
  hash_table_node_t *node;

  node = (hash_table_node_t *)malloc(sizeof(hash_table_node_t));
  if(node == NULL)
  {
    fprintf(stderr, "allocate_hash_table_node: out of memory\n");
    exit(1);
  }
  return node;
}

static void free_hash_table_node(hash_table_node_t *node)
{
  adjacency_node_t *adj_crawler = node->head;
  adjacency_node_t *adj_before;
  while(adj_crawler != NULL)
  {
    adj_before = adj_crawler;
    adj_crawler = adj_crawler->next;
    free_adjacency_node(adj_before);
  }
  free_adjacency_node(adj_crawler);

  free(node);
}

//
// hash table stuff (mostly to be done)
//
unsigned int crc32(const char *str)
{
  static unsigned int table[256];
  unsigned int crc;

  if(table[1] == 0u) // do we need to initialize the table[] array?
  {
    unsigned int i,j;

    for(i = 0u;i < 256u;i++)
      for(table[i] = i,j = 0u;j < 8u;j++)
        if(table[i] & 1u)
          table[i] = (table[i] >> 1) ^ 0xAED00022u; // "magic" constant
        else
          table[i] >>= 1;
  }
  crc = 0xAED02022u; // initial value (chosen arbitrarily)
  while(*str != '\0')
    crc = (crc >> 8) ^ table[crc & 0xFFu] ^ ((unsigned int)*str++ << 24);
  return crc;
}

static hash_table_t *hash_table_create(void)
{
  hash_table_t *hash_table;

  hash_table = (hash_table_t *)malloc(sizeof(hash_table_t));
  if(hash_table == NULL) {
    fprintf(stderr,"create_hash_table: out of memory\n");
    exit(1);
  }

  hash_table->hash_table_size = 200;
  hash_table->number_of_entries = 0;
  hash_table->number_of_edges = 0;

  hash_table->heads = (unsigned int*) malloc(hash_table->hash_table_size*sizeof(unsigned int*));  
  memset(hash_table->heads, NULL, hash_table->hash_table_size*sizeof(hash_table->heads));

  return hash_table;
}

static hash_table_node_t *find_word(hash_table_t **hash_table,const char *word,int insert_if_not_found);
static void hash_table_free(hash_table_t *hash_table);

static void hash_table_grow(hash_table_t *hash_table)
{
  unsigned int hashVal;  
  hash_table_node_t *node;
  hash_table_node_t *next_node;

  hash_table->hash_table_size = hash_table->hash_table_size * 2;
  totalGrows++;
  // Alocar o novo array de hash table
  hash_table_node_t **new_heads = (unsigned int*) malloc(hash_table->hash_table_size*sizeof(unsigned int*));  
  memset(new_heads, NULL, hash_table->hash_table_size*sizeof(unsigned int*));
  
  // Iterar sobre a hash table antiga
  for (unsigned int i = 0u; i < hash_table->hash_table_size/2; i++) { 
    if (hash_table->heads[i] != NULL) {   
      node = hash_table->heads[i];
      // Iterar sobre as linked lists
      while(node != NULL) {
        next_node = node->next;
        hashVal = crc32(node->word) % hash_table->hash_table_size;

        // Colocar na nova hash table
        if (new_heads[hashVal] == NULL) {
          node->next = NULL;
          new_heads[hashVal] = node;
        }
        else {
          hash_table_node_t *last_node;

          last_node = new_heads[hashVal];
          while (last_node->next != NULL) {
            last_node = last_node->next;
          }
          last_node->next = node;
          node->next = NULL;
        }

        node = next_node;
      }
    }
  }
  // Trocar o array na variável hash_table_t pelo novo array
  hash_table_node_t **old_heads = hash_table->heads;
  hash_table->heads = new_heads;
  free(old_heads);
}

static void hash_table_free(hash_table_t *hash_table)
{
  hash_table_node_t *crawler;
  hash_table_node_t *node_before;
  // Percorrer a hash table toda
  for (unsigned int i = 0u; i < hash_table->hash_table_size; i++) {
    if (hash_table->heads[i] == NULL) {
      continue;
    }
    // Percorrer pelas listas até ao final
    crawler = hash_table->heads[i];
    while (crawler->next != NULL) {
      node_before = crawler;
      crawler = crawler->next;
      // Libertar o node anterior ao crawler
      free_hash_table_node(node_before);
    }
    // Libertar o node deixado para trás pelo crawler
    free_hash_table_node(crawler);
  }
  // Libertar o resto da hash table
  free(hash_table->heads);
  free(hash_table);
  
  return;
}

static hash_table_node_t *find_word(hash_table_t **hash_table,const char *word,int insert_if_not_found)
{
  unsigned int hashVal;

  // Verificar o preenchimento da hash table
  if ((*hash_table)->number_of_entries >= (*hash_table)->hash_table_size / 2) {
    hash_table_grow(*hash_table);
  }
  // Obter o hash code da palavra
  hashVal = crc32(word) % (*hash_table)->hash_table_size;

  // Se a operação for de insert
  if (insert_if_not_found == 1) {
    // Criar o Node
    hash_table_node_t *node = allocate_hash_table_node();
      node->next = NULL;
      node->head = NULL;
      node->visited = 0;
      node->representative = node;
      node->number_of_edges = 0;
      node->number_of_vertices = 1;

    // Se não existir um Node nesse hash value 
    if ((*hash_table)->heads[hashVal] == NULL) {
      strcpy(node->word, word);

      (*hash_table)->heads[hashVal] = node;
      (*hash_table)->number_of_entries++;
    }
    // Se já existir um Node nesse hash value 
    else {
      totalColisions++;
      hash_table_node_t *last_node;
      // Percorrer a lista até ao fim
      last_node = (*hash_table)->heads[hashVal];
      while (last_node->next != NULL)
      {
        last_node = last_node->next;
      }
      last_node->next = node;
      strcpy(node->word, word);
    }
    return NULL;
  }

  // Caso não seja para inserir nodes novos
  else {
    hash_table_node_t *node;
    // Caso o hash value corresponda a algum node
    if ((*hash_table)->heads[hashVal] != NULL) {
      node = (*hash_table)->heads[hashVal];
      // Percorrer a lista de nodes
      // até encontrar o pretendido
      while (node != NULL) {      
        if (strcmp(node->word, word)==0) {
          return node;
        }
        else {
          node = node->next;
        }
      }
    }
    return NULL;
  }
}


//
// add edges to the word ladder graph (mostly do be done)
//
static hash_table_node_t *find_representative(hash_table_node_t *node)
{
  hash_table_node_t *representative,*next_node;
  representative = node;
  // Mover para o node representatívo até um deles apontar para sí próprio
  while (representative->representative != representative) {
    representative = representative->representative;    
  }  
  // Optimizar o node, apontando diretaente para o node representatívo real
  node->representative = representative;
  return representative;
}

static void add_edge(hash_table_t *hash_table,hash_table_node_t *from,const char *word)
{
  hash_table_node_t *to,*from_representative,*to_representative;
  adjacency_node_t *link;
  to = find_word(&hash_table,word,0);

  if (to == NULL) {
    return;
  }

  from->number_of_edges++;
  to->number_of_edges++;

  // Encontrar o representativo de cada node
  from_representative = find_representative(from);
  to_representative = find_representative(to);
  totalEdges++;

  if (from_representative != to_representative) {
    //  Comparar os componentes conexos de cada node para decidir qual 
    // componente conexo prevalece na sua junção
    if (from_representative->number_of_vertices > to_representative->number_of_vertices) {
      from_representative->number_of_vertices += to_representative->number_of_vertices;
      to_representative->representative = from_representative;
      to->representative = from_representative;
    }
    //  Se forem menores ou iguais, vai para o node menor ou o que tiver mais valor 
    // em strcmp(sempre o to)
    else if (from_representative->number_of_vertices < to_representative->number_of_vertices) {
      to_representative->number_of_vertices += from_representative->number_of_vertices;
      from_representative->representative = to_representative;
      from->representative = to_representative;
    }
    //  Automaticamente atribui a prioridade à palavra com menor valor no strcmp, 
    // visto que só essas são usadas nesta função
    else {
      from_representative->number_of_vertices += to_representative->number_of_vertices;
      to_representative->representative = from_representative;
      to->representative = from_representative;
    }
  }


  // Adicionar link ao node from
  adjacency_node_t *new_link0 = allocate_adjacency_node();
  new_link0->vertex = to;
  new_link0->next = NULL;
  link = from->head;
  to->visited = 1;
  // Colocar na lista de links do node
  if(link == NULL) {
    from->head = new_link0;
  }
  else {
    while(link->next != NULL) {
      link = link->next;
    }
    link->next = new_link0;
  }

  // Adicionar link ao node to
  adjacency_node_t *new_link1 = allocate_adjacency_node();
  new_link1->vertex = from;
  new_link1->next = NULL;
  link = to->head;
  // Colocar na lista de links do node
  if(link == NULL) {
    to->head = new_link1;
  }
  else {
    while(link->next != NULL) {      
      link = link->next;
    }
    link->next = new_link1;
  }

  return;
}


//
// generates a list of similar words and calls the function add_edge for each one (done)
//
// man utf8 for details on the uft8 encoding
//
static void break_utf8_string(const char *word,int *individual_characters)
{
  int byte0,byte1;

  while(*word != '\0')
  {
    byte0 = (int)(*(word++)) & 0xFF;
    if(byte0 < 0x80)
      *(individual_characters++) = byte0; // plain ASCII character
    else
    {
      byte1 = (int)(*(word++)) & 0xFF;
      if((byte0 & 0b11100000) != 0b11000000 || (byte1 & 0b11000000) != 0b10000000)
      {
        fprintf(stderr,"break_utf8_string: unexpected UFT-8 character\n");
        exit(1);
      }
      *(individual_characters++) = ((byte0 & 0b00011111) << 6) | (byte1 & 0b00111111); // utf8 -> unicode
    }
  }
  *individual_characters = 0; // mark the end!
}

static void make_utf8_string(const int *individual_characters,char word[_max_word_size_])
{
  int code;

  while(*individual_characters != 0)
  {
    code = *(individual_characters++);
    if(code < 0x80)
      *(word++) = (char)code;
    else if(code < (1 << 11))
    { // unicode -> utf8
      *(word++) = 0b11000000 | (code >> 6);
      *(word++) = 0b10000000 | (code & 0b00111111);
    }
    else
    {
      fprintf(stderr,"make_utf8_string: unexpected UFT-8 character\n");
      exit(1);
    }
  }
  *word = '\0';  // mark the end
}

static void similar_words(hash_table_t *hash_table,hash_table_node_t *from)
{
  static const int valid_characters[] =
  { // unicode!
    0x2D,                                                                       // -
    0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,           // A B C D E F G H I J K L M
    0x4E,0x4F,0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,           // N O P Q R S T U V W X Y Z
    0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,           // a b c d e f g h i j k l m
    0x6E,0x6F,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,           // n o p q r s t u v w x y z
    0xC1,0xC2,0xC9,0xCD,0xD3,0xDA,                                              // Á Â É Í Ó Ú
    0xE0,0xE1,0xE2,0xE3,0xE7,0xE8,0xE9,0xEA,0xED,0xEE,0xF3,0xF4,0xF5,0xFA,0xFC, // à á â ã ç è é ê í î ó ô õ ú ü
    0
  };
  int i,j,k,individual_characters[_max_word_size_];
  char new_word[2 * _max_word_size_];

  break_utf8_string(from->word,individual_characters);
  for(i = 0;individual_characters[i] != 0;i++)
  {
    k = individual_characters[i];
    for(j = 0;valid_characters[j] != 0;j++)
    {
      individual_characters[i] = valid_characters[j];
      make_utf8_string(individual_characters,new_word);
      // avoid duplicate cases
      if(strcmp(new_word,from->word) > 0){
        add_edge(hash_table,from,new_word);
      }
    }
    individual_characters[i] = k;
  }
  if (from->head == NULL) {
    return;
  }
  for(adjacency_node_t *link = from->head; link->next != NULL; link = link->next) {
    link->vertex->visited = 0;
  }
}


//
// breadth-first search (to be done)
//
// returns the number of vertices visited; if the last one is goal, following the previous links gives the shortest path between goal and origin
//
static int breadth_first_search(int maximum_number_of_vertices,hash_table_node_t **list_of_vertices,hash_table_node_t *origin,hash_table_node_t *goal)
{
  //
  // complete this
  //
  return -1;
}


//
// list all vertices belonging to a connected component (complete this)
//
static void list_connected_component(hash_table_t *hash_table,const char *word, int numSpaces)
{
  hash_table_node_t *node = find_word(&hash_table, word, 0);
  // Caso a palavra não exista
  if (node == NULL) {  
    printf("                     │            ERRO!!!            │\n", word);
    printf("                     │    Essa palavra não existe    │\n", word);
    printf("                     │   no ficheiro selecionado!    │\n", word);
    return;
  }
  // Caso tenha-mos chegado ao fim da lista de links da palavra
  if (node->head == NULL) {
    return;
  }

  // Marcar o node atual como visitado
  node->visited = 1;
  // Iterar sobre todas os nodes ligados ao original
  for(adjacency_node_t *link = node->head; link!= NULL; link = link->next) {
    // Não listar nodes visitados
    if (link->vertex->visited == 1) {
      continue;
    }
    // Imprimir o node
    printf("                     │ Nivel %4i │  %14s  │\n", numSpaces ,link->vertex->word);
    // Recursívamente ler o próximo node e os seus links
    list_connected_component(hash_table, link->vertex->word, numSpaces+1);
  }
}


//
// compute the diameter of a connected component (optional)
//
static int largest_diameter;
static hash_table_node_t **largest_diameter_example;

static int connected_component_diameter(hash_table_node_t *node)
{
  int diameter;

  //
  // complete this
  //
  return diameter;
}


//
// find the shortest path from a given word to another given word (to be done)
//
static void path_finder(hash_table_t *hash_table,const char *from_word,const char *to_word)
{
  const hash_table_node_t * source = find_word(&hash_table, from_word, 0) ,
                          * goal = find_word(&hash_table, to_word, 0) ,
                          * currentPath ,
                          * allPaths;
  adjacency_node_t * link ; hash_table_node_t * node, * currentVertex;

  int goal_found = 0 , pathn = 0;

  //Iterate through every adjacency node
  for (link = source->head ; link != NULL ; link->next) {
    fprintf(stderr, "\n--> ");


    //Dont check the path that is already travaled by
    if( currentVertex->visited == 1 ){continue ;}

    currentVertex->visited = 1 ;


    // Get the actual vertex
    //currentVertex = link->vertex ;
    for(currentVertex = link->vertex ; currentVertex != NULL ; currentVertex->next){
      printf(" | %s | ", currentVertex->word);

      if( strcmp ( currentVertex->word ,  goal->word ) == 0 ){
          goal_found = 0;
          fprintf(stderr, "\n<-- ");
          break;
       }

    }
  }
}


//
// some graph information (optional)
//
static void graph_info(hash_table_t *hash_table)
{
  printf("             ╭─────────────────────────────────────────────────────╮\n");
  printf("             │                     Graph Info                      │\n");
  printf("             ├───────────────────────────────────────────┬─────────┤\n");
  printf("             │ Number of Edges                           │ %7i │\n", totalEdges);
  printf("             │ Number of Vertices                        │ %7i │\n", totalWords);
  printf("             │ Number of connected components (graphs)   │ %7i │\n", numConnectedComponents);
  printf("             │ Number of Words with no connections       │ %7i │\n", numSeperatedComponents);
  printf("             │ Size of the largest connected component   │ %7i │\n", largestComponent);
  printf("             ╰───────────────────────────────────────────┴─────────╯\n");
}


//
// some hash table information
//
static void hash_table_info(hash_table_t *hash_table)
{
  printf("             ╭─────────────────────────────────────────────────────╮\n");
  printf("             │                  Hash Table Info                    │\n");
  printf("             ├───────────────────────────────────────────┬─────────┤\n");
  printf("             │ Number of Words                           │ %7i │\n", totalWords);
  printf("             │ Number of Colisions                       │ %7i │\n", totalColisions);
  printf("             │ Number of uncolided words                 │ %7i │\n", totalWords - totalColisions);
  printf("             │ Percentage of Colisions                   │ %6i\% │\n", 100*totalColisions/totalWords);
  printf("             ├───────────────────────────────────────────┼─────────┤\n");
  printf("             │ Node with the most Colisions              │ %7i │\n", mostColHashNode);
  printf("             │ Most Colisions in that Node               │ %7i │\n", mostColisions);
  printf("             ├───────────────────────────────────────────┼─────────┤\n");
  printf("             │ Final size of the Hash Table              │ %7i │\n", hash_table->hash_table_size);
  printf("             │ Number of Nodes used                      │ %7i │\n", numUsedHashNodes);
  printf("             │ Number of empty Nodes                     │ %7i │\n", hash_table->hash_table_size - numUsedHashNodes);
  printf("             │ Percentage of Hash Table Nodes used       │ %6i\% │\n", 100*numUsedHashNodes/hash_table->hash_table_size);
  printf("             │ Number of Hash Table grows                │ %7i │\n", totalGrows);
  printf("             ╰───────────────────────────────────────────┴─────────╯\n");
}

//
// calculate the hash table info, the Hash Node with the most colisions, number of nodes used, etc
//
static void calculateInfo(hash_table_t *hash_table, int* mostColHashNode, int* mostColisions, int* numUsedHashNodes, int* numConnectedComponents, int* numSeperatedComponents, int* largestComponent)
{
  hash_table_node_t *node;

  // Run through every hash table head
  for (int x = 0u; x <= hash_table->hash_table_size; x++) {
    if(hash_table->heads[x] == NULL) {
      continue;
    }
   
    // Increment number of used nodes
    *numUsedHashNodes = *numUsedHashNodes + 1;

    // Calculate how many nodes the head has
    int nodesInX = 0;
    for(node = hash_table->heads[x];node != NULL;node = node->next) {
      nodesInX++;

      // Calculate the number of representatives (same as number of connected components)
      hash_table_node_t* representative = find_representative(node);
      if (representative->visited == 0) {
        *numConnectedComponents = *numConnectedComponents + 1;
        representative->visited = 1;
        // Calculate the largest component (most vertices)
        //printf("\nLargestComp > %i", representative->number_of_vertices);
        if (representative->number_of_vertices >= *largestComponent) {
          *largestComponent = representative->number_of_vertices; 
        }
        // Calculate number of connected components with only one word (unconnected)
        if (representative->number_of_vertices < 2) {
          *numSeperatedComponents = *numSeperatedComponents + 1;
        }
      }

    }
    // Compare the number of nodes to the maximum known
    if (nodesInX > *mostColisions) {
      *mostColHashNode = x;
      *mostColisions = nodesInX;
    }
  }

  // Set all the nodes's "visited" flag to 0
  setNodesVisitedTo0(hash_table);
  return;
}


//
// main program
//
void progressBar(int percent) {
    printf("\r ├");
    for (int x = 0; x < percent-1; x++) {
      printf("■");
    }
    printf("▶");
    for (int x = percent; x < 100; x++) {
      printf("─");
    }
    printf("┤├%3i%┤ ", percent);
    fflush(stdout);
}

void setNodesVisitedTo0(hash_table_t *hash_table) {
  hash_table_node_t *node;
  // Set all the node's "visited" flag back to 0
  for (int x = 0u; x <= hash_table->hash_table_size; x++) {
    if(hash_table->heads[x] == NULL) {
      continue;
    }
    for(node = hash_table->heads[x];node != NULL;node = node->next) {
      node->visited = 0;
    }
  }
}

int main(int argc,char **argv)
{
  char word[100],from[100],to[100];
  hash_table_t *hash_table;
  hash_table_node_t *node;
  unsigned int i;
  int command;
  FILE *fp;

  // initialize hash table
  hash_table = hash_table_create();

  // read words
  fp = fopen((argc < 2) ? "wordlist-big-latest.txt" : argv[1],"rb");
  if(fp == NULL) {
    fprintf(stderr,"main: unable to open the words file\n");
    exit(1);
  }
  
  int percent=0;

  printf("\n  Filling up the hash table...\n");
  while(fscanf(fp,"%99s",word) == 1) {
    (void)find_word(&hash_table,word,1);
    totalWords++;
    percent = (int) (hash_table->number_of_entries * 100 / (hash_table->hash_table_size / 2));
    progressBar(percent);
  }
  fclose(fp);

  percent = 100;
  progressBar(percent);
  printf("\n");
  percent = 0;

  printf("\n  Connecting all the nodes...\n");
  // Iterar sobre todos os nodes
  for(i = 0u;i < hash_table->hash_table_size;i++) {
    percent = (int) (i * 100 / hash_table->hash_table_size);
    progressBar(percent);
    for(node = hash_table->heads[i];node != NULL;node = node->next) {
      similar_words(hash_table,node);
    }
  }

  // Set all the nodes's "visited" flag to 0
  setNodesVisitedTo0(hash_table);

  calculateInfo(hash_table, &mostColHashNode, &mostColisions, &numUsedHashNodes, &numConnectedComponents, &numSeperatedComponents, &largestComponent);


  percent = 100;
  progressBar(percent);
  printf("\n");

  // ask what to do
  for(;;)
  {
    printf("\n\n");
    fprintf(stderr," ╭──────────────────────────────────────────────────────────────────────────────╮\n");
    fprintf(stderr," │                           Your wish is my command:                           │  \n");
    fprintf(stderr," ╰──────────────────────────────────────────────────────────────────────────────╯\n");
    fprintf(stderr," ╭───┬─────────────────────────┬────────────────────────────────────────────────╮\n");
    fprintf(stderr," │ 1 │ WORD                    │  list the connected component WORD belongs to  │\n");
    fprintf(stderr," │ 2 │ FROM TO                 │  list the shortest path from FROM to TO        │\n");
    fprintf(stderr," │ 3 │ DISPLAY HASH TABLE      │  display the hash table and apropriate info    │\n");
    fprintf(stderr," │ 4 │ DISPLAY HASH TABLE INFO │  display apropriate info about the hash table  │\n");
    fprintf(stderr," │ 5 │ DISPLAY GRAPH           │  display the graph                             │\n");
    fprintf(stderr," │ 6 │ DISPLAY GRAPH INFO      │  display apropriate info about the graph       │\n");
    fprintf(stderr," │ 7 │                         │  terminate                                     │\n");
    fprintf(stderr," ╰───┴─────────────────────────┴────────────────────────────────────────────────╯\n");
    fprintf(stderr,"                                    -> ");
    if(scanf("%99s",word) != 1)
      break;
    command = atoi(word);
    //system("clear");

    if(command == 1) {
      if(scanf("%99s",word) != 1)
        break;
      printf("\n                         ╭───────────────────────╮\n");
      printf("                         │ -> %-18s │\n", word);
      printf("                         ╰───────────────────────╯\n");
      printf("                     ╭────────────┬──────────────────╮\n");
      list_connected_component(hash_table,word, 0);
      printf("                     ╰────────────┴──────────────────╯\n");
    }

    else if(command == 2) {
      if(scanf("%99s",from) != 1)
        break;
      if(scanf("%99s",to) != 1)
        break;
      path_finder(hash_table,from,to);
    }

    else if(command == 3) {
      printf("         ╭───────────╮");
      // Iterar sobre a tabela toda
      for (int x = 0u; x < hash_table->hash_table_size; x++) {
        printf("         │ [%7i] │", x);
        // Iterar sobre as listas
        for(node = hash_table->heads[x];node != NULL;node = node->next) {
          printf(" -> %s", node->word);
        }
        printf("\n");
      }

      printf("         ╰───────────╯");
    }

    else if(command == 4) {
      hash_table_info(hash_table);
    }

    else if(command == 5) {
      int minConnCompSize;
      int numConnCompShowed = 0;
      char* tempStr[100];
      fprintf(stderr,"            Tamanho mínimo do grafo -> ");
      scanf("%99s",tempStr) != 1;
      minConnCompSize = atoi(tempStr);
      // Print the nodes of all the representatives
      for (int x = 0u; x <= hash_table->hash_table_size; x++) {
        if(hash_table->heads[x] == NULL) {
          continue;
        }
        for(node = hash_table->heads[x];node != NULL;node = node->next) {
          hash_table_node_t* representative = find_representative(node);
          if (representative->visited == 0 && representative->number_of_vertices >= minConnCompSize) {
            printf("\n                         ╭───────────────────────╮\n");
            printf("                         │    Representative:    │\n");
            printf("                         │ -> %-18s │\n", representative->word);
            printf("                         ╰───────────────────────╯\n");
            printf("                     ╭────────────┬──────────────────╮\n");
            list_connected_component(hash_table, representative->word, 0);
            printf("                     ╰────────────┴──────────────────╯\n");
            numConnCompShowed++;
            representative->visited = 1;
          }
        }
      }

      printf("\n                       ╭───────────────────────────╮\n");
      printf("                       │   Número de componentes   │\n");
      printf("                       │ conexos com tamanho igual │\n");
      printf("                       │    ou maior do que %-4i   │\n", minConnCompSize);
      printf("                       ├───────────────────────────┤\n");
      printf("                       │  -> %9i             │\n", numConnCompShowed);
      printf("                       ╰───────────────────────────╯\n");
    }
    else if(command == 6) {
      graph_info(hash_table);
    }

    else if(command == 7)
      break;

    
    setNodesVisitedTo0(hash_table);
  }

  // clean up
  hash_table_free(hash_table);
  return 0;
}
