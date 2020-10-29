// Copyright 2020 Voicu Ana-Nicoleta
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

union container {
  char charptr[97];
  struct metadate {
    char tag[3];
    char titlu[30];
    char artist[30];
    char album[30];
    char an[5];
  } metadate;
} container;

struct Node {
  union container data;
  struct Node *next;
  struct Node *prev;
};

struct List {
  struct Node *head;
  struct Node *tail;
  struct Node *cursor;
  int size;
};

void init_list(struct List *list) {
  list->head = list->tail = NULL;
  list->cursor = NULL;
  list->size = 0;
}

void get_path(FILE *fin, char path[256]) {
  FILE *fsong;
  int x = 256;
  char nume_melodie[100];
  fseek(fin, 1, SEEK_CUR);
  fgets(nume_melodie, 100, fin);
  nume_melodie[strlen(nume_melodie) - 1] = '\0';
  snprintf(path, x, "./songs/%s", nume_melodie);
}

int del_first(struct List *list) {
  struct Node *extract;
  if (list->size == 0)
    return 0;
  extract = list->head;
  if (list->size == 1) {
    list->head = list->tail = list->cursor = NULL;
    list->size--;
  }
  else if (list->size > 1)
  {
    if (list->cursor == list->head)
      list->cursor = list->cursor->next;
    list->head = list->head->next;
    list->head->prev = NULL;
    list->size--;
  }
  free(extract);
  return 1;
}

int del_last(struct List *list) {
  struct Node *extract;
  if (list->size == 0)
    return 0;
  extract = list->tail;
  if (list->size == 1) {
    list->head = list->tail = list->cursor = NULL;
    list->size--;
  }
  else if (list->size > 1)
  {
    if (list->cursor == list->tail)
      list->cursor = list->cursor->prev;
    list->tail = list->tail->prev;
    list->tail->next = NULL;
    list->size--;
  }
  free(extract);
  return 1;
}

void del_curr(struct List *list, FILE *fout) {
  struct Node *extract;
  extract = list->cursor;
  if (list->size == 0)
    fprintf(fout, "Error: no track playing\n");
  else if (list->size == 1)
  {
    list->head = list->tail = list->cursor = NULL;
    list->size--;
    free(extract);
  }
  else if (list->size > 1)
  {
    if (list->cursor == list->head)
      del_first(list);
    else if (list->cursor == list->tail)
      del_last(list);
    else
    {
      list->cursor->prev->next = list->cursor->next;
      list->cursor->next->prev = list->cursor->prev;
      list->cursor = list->cursor->next;
      list->size--;
      free(extract);
    }
  }
}

int del_song(struct List *list, FILE *fin, FILE *fout) {
  int i;
  struct Node *extract;
  FILE *fsong;
  char nume_melodie[100];
  char path[256];
  fseek(fin, 1, SEEK_CUR);
  fgets(nume_melodie, 75, fin);
  nume_melodie[strlen(nume_melodie) - 1] = '\0';
  snprintf(path, sizeof(path) + 10*sizeof(char), "%s", "./songs/");
  snprintf(path + strlen(path), sizeof(path) + sizeof(nume_melodie),
  "%s", nume_melodie);
  fsong = fopen(path, "rb");
  if (fsong != NULL && list->size > 0) {
    fseek(fsong, -97, SEEK_END);
    fread(container.charptr, sizeof(char), 97, fsong);
    snprintf(nume_melodie, sizeof(path) + sizeof(nume_melodie),
    "%s", container.metadate.titlu);
    extract = list->head;
    for (i = 0; i < list->size; i++) {
      if (!strcmp(nume_melodie, extract->data.metadate.titlu)) {
        if (extract == list->head)
          del_first(list);
        else if (extract == list->tail)
          del_last(list);
        else
        {
          extract->prev->next = extract->next;
          extract->next->prev = extract->prev;
          list->size--;
          free(extract);
        }
        fclose(fsong);
        return 0;
      }
      extract = extract->next;
    }
  }
  fclose(fsong);
  fprintf(fout, "Error: no song found to delete\n");
}

void add_first(struct List *list, FILE *fin) {
  int i, ok = 0;
  struct Node *node = NULL;
  struct Node *traverse = NULL;
  FILE *fsong;
  char path[256];
  node = calloc(1, sizeof(struct Node));
  get_path(fin, path);
  fsong = fopen(path, "rb");
  if (fsong != NULL) {
    fseek(fsong, -97, SEEK_END);
    fread(container.charptr, sizeof(char), 97, fsong);
    node->data = container;
    snprintf(path, sizeof(path) + sizeof(node->data.metadate.titlu),
    "%s", node->data.metadate.titlu);
    traverse = list->head;
    for (i = 0; i < list->size; i++) {
      if (!strcmp(path, traverse->data.metadate.titlu)) {
        ok = 1;
        if (traverse == list->head)
          del_first(list);
        else if (traverse == list->tail)
          del_last(list);
        else
        {
          if (traverse == list->cursor)
            list->cursor = list->cursor->next;
          traverse->prev->next = traverse->next;
          traverse->next->prev = traverse->prev;
          list->size--;
          free(traverse);
        }
        break;
      }
      traverse = traverse->next;
    }
    if (ok == 0)
      free(traverse);

    // tratam cazul cand lista e goala
    if (list->size == 0) {
      node->next = NULL;
      node->prev = NULL;
      list->head = list->tail = list->cursor = node;
    } else {
      list->head->prev = node;
      node->next = list->head;
      node->prev = NULL;
      list->head = node;
    }
    list->size++;
    fclose(fsong);
  }
}

void add_last(struct List *list, FILE *fin) {
  struct Node *node;
  struct Node *traverse;
  int i, ok = 0;
  FILE *fsong;
  char path[256];
  get_path(fin, path);
  fsong = fopen(path, "rb");
  if (fsong != NULL) {
    fseek(fsong, -97, SEEK_END);
    fread(container.charptr, sizeof(char), 97, fsong);
    node = calloc(1, sizeof(struct Node));
    node->data = container;
    snprintf(path, sizeof(path) + sizeof(node->data.metadate.titlu),
    "%s", node->data.metadate.titlu);
    traverse = list->head;

    for (i = 0; i < list->size; i++) {
      if (!strcmp(path, traverse->data.metadate.titlu)) {
        ok = 1;
        if (traverse == list->head)
          del_first(list);
        else if (traverse == list->tail)
          del_last(list);
        else
        {
          if (traverse == list->cursor)
            list->cursor = list->cursor->next;
          traverse->prev->next = traverse->next;
          traverse->next->prev = traverse->prev;
          list->size--;
          free(traverse);
        }
        break;
      }
      traverse = traverse->next;
    }
    if (ok == 0)
      free(traverse);
    // tratam cazul cand lista e goala
    if (list->size == 0) {
      node->next = NULL;
      node->prev = NULL;
      list->head = list->tail = list->cursor = node;
    } else {
      list->tail->next = node;
      node->next = NULL;
      node->prev = list->tail;
      list->tail = node;
    }
    list->size++;
    fclose(fsong);
  }
}

void add_after(struct List *list, FILE *fin) {
  struct Node *traverse = NULL;
  struct Node *node = NULL;
  FILE *fsong;
  int i, ok = 0;
  char path[256];
  get_path(fin, path);
  fsong = fopen(path, "rb");
  if (fsong != NULL && list->size > 0) {
    fseek(fsong, -97, SEEK_END);
    fread(container.charptr, sizeof(char), 97, fsong);
    node = calloc(1, sizeof(struct Node));
    node->data = container;

    snprintf(path, sizeof(node->data.metadate.titlu) + sizeof(path),
    "%s", node->data.metadate.titlu);
    traverse = list->head;
    for (i = 0; i < list->size; i++) {
      if (!strcmp(path, traverse->data.metadate.titlu) &&
          traverse != list->cursor) {
        ok = 1;
        if (traverse == list->head)
          del_first(list);
        else if (traverse == list->tail)
          del_last(list);
        else
        {
          if (traverse == list->cursor)
            list->cursor = list->cursor->next;
          traverse->prev->next = traverse->next;
          traverse->next->prev = traverse->prev;
          list->size--;
          free(traverse);
        }
        break;
      }
      traverse = traverse->next;
    }
    if (ok == 0)
      free(traverse);
    if (strcmp(node->data.metadate.titlu, list->cursor->data.metadate.titlu)) {
      if (list->cursor == list->tail) {
        list->cursor->next = node;
        node->prev = list->cursor;
        node->next = NULL;
        list->tail = node;
      } else {
        node->next = list->cursor->next;
        list->cursor->next->prev = node;
        list->cursor->next = node;
        node->prev = list->cursor;
      }
      list->size++;
    }
    else
      free(node);
  }
  fclose(fsong);
}

void move_next(struct List *list, FILE *fout) {
  if (list->size == 0)
    fprintf(fout, "Error: no track playing\n");
  else if (list->cursor->next != NULL)
  {
    list->cursor = list->cursor->next;
  }
}

void move_prev(struct List *list, FILE *fout) {
  if (list->size == 0)
    fprintf(fout, "Error: no track playing\n");
  else if (list->cursor->prev != NULL)
  {
    list->cursor = list->cursor->prev;
  }
}

void show_first(struct List *list, FILE *fout) {
  if (list->size == 0)
    fprintf(fout, "Error: show empty playlist\n");
  else
  {
    fprintf(fout, "Title: %.30s\n", list->head->data.metadate.titlu);
    fprintf(fout, "Artist: %.30s\n", list->head->data.metadate.artist);
    fprintf(fout, "Album: %.30s\n", list->head->data.metadate.album);
    fprintf(fout, "Year: %s\n", list->head->data.metadate.an);
  }
}

void show_last(struct List *list, FILE *fout) {
  if (list->size == 0)
    fprintf(fout, "Error: show empty playlist\n");
  else
  {
    fprintf(fout, "Title: %.30s\n", list->tail->data.metadate.titlu);
    fprintf(fout, "Artist: %.30s\n", list->tail->data.metadate.artist);
    fprintf(fout, "Album: %.30s\n", list->tail->data.metadate.album);
    fprintf(fout, "Year: %s\n", list->tail->data.metadate.an);
  }
}

void show_curr(struct List *list, FILE *fout) {
  if (list->size == 0)
    fprintf(fout, "Error: show empty playlist\n");
  else
  {
    fprintf(fout, "Title: %.30s\n", list->cursor->data.metadate.titlu);
    fprintf(fout, "Artist: %.30s\n", list->cursor->data.metadate.artist);
    fprintf(fout, "Album: %.30s\n", list->cursor->data.metadate.album);
    fprintf(fout, "Year: %s\n", list->cursor->data.metadate.an);
  }
}

void show_playlist(struct List *list, FILE *fout) {
  int i;
  struct Node *traverse = NULL;
  if (list->size == 0)
    fprintf(fout, "[]");
  else
  {
    traverse = list->head;
    fprintf(fout, "[");
    for (i = 0; i < list->size - 1; i++) {
      fprintf(fout, "%.30s; ", traverse->data.metadate.titlu);
      traverse = traverse->next;
    }
    fprintf(fout, "%.30s]", traverse->data.metadate.titlu);
  }
}

void free_list(struct List *list) {
	int i;
	struct Node* curent;
	struct Node* traverse;
	curent = list->head;
	traverse = curent;
	for(i = 0; i < list->size; i++) {
		traverse = curent->next;
		free(curent);
		curent = traverse;
	}
	free(list);
}

int main(int argc, char *argv[]) {
  struct List *list;
  int numar_comenzi, i;
  char comanda[512] = {0};

  list = malloc(sizeof(struct List));
  init_list(list);

  FILE *fin = fopen(argv[1], "r");
  if (fin == NULL) {
    fprintf(stderr, "File %s is not readable.\n", argv[1]);
    return -1;
  }

  FILE *fout = fopen(argv[2], "w");
  if (fout == NULL) {
    fprintf(stderr, "File %s is not readable.\n", argv[2]);
    return -1;
  }

  fscanf(fin, "%d", &numar_comenzi);
  for (i = 0; i < numar_comenzi; i++) {
    fscanf(fin, "%s", comanda);
    if (!strcmp(comanda, "ADD_FIRST"))
      add_first(list, fin);
    else if (!strcmp(comanda, "ADD_LAST"))
      add_last(list, fin);
    else if (!strcmp(comanda, "ADD_AFTER"))
      add_after(list, fin);
    else if (!strcmp(comanda, "SHOW_PLAYLIST"))
      show_playlist(list, fout);
    else if (!strcmp(comanda, "DEL_FIRST"))
    {
      if (del_first(list) == 0)
        fprintf(fout, "Error: delete from empty playlist\n");
    }
    else if (!strcmp(comanda, "DEL_LAST"))
    {
      if (del_last(list) == 0)
        fprintf(fout, "Error: delete from empty playlist\n");
    }
    else if (!strcmp(comanda, "DEL_CURR"))
      del_curr(list, fout);
    else if (!strcmp(comanda, "DEL_SONG"))
      del_song(list, fin, fout);
    else if (!strcmp(comanda, "MOVE_NEXT"))
      move_next(list, fout);
    else if (!strcmp(comanda, "MOVE_PREV"))
      move_prev(list, fout);
    else if (!strcmp(comanda, "SHOW_FIRST"))
      show_first(list, fout);
    else if (!strcmp(comanda, "SHOW_LAST"))
      show_last(list, fout);
    else if (!strcmp(comanda, "SHOW_CURR"))
      show_curr(list, fout);
  }
  fclose(fout);
  fclose(fin);
  free_list(list);
  return 0;
}
