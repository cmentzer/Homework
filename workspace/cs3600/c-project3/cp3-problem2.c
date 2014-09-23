/*
 * CS3600, Fall 2013
 * C Bootcamp, Homework 3, Problem 2
 * (c) 2012 Alan Mislove
 *
 * In this problem, your goal is to write a simple library that
 * implements a ordered doubly-linked list.  The list elements 
 * will therefore consist of pairs (int, string).  You must ensure that 
 * there is only one element with a given int value, and you must support
 * lookup based on the int.  *You should store the elements in the list
 * in numerical order based on the ints, from lowest to highest.*
 *
 * You are required to fill in the functions to manipulate the lists and
 * list element structure (typedef-ed if you wish), and use dynamic 
 * memory allocation (malloc/calloc) to add elements to the list
 * (i.e., allocate new list element structures).  You must also be
 * sure to free() any memory when list elements are deleted.
 *
 * Do not touch anything in the main() function, and do not modify the
 * the list structures.
 */

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

/**
 * The list elements
 */
struct double_linked_list_element_s;

typedef struct double_linked_list_element_s {
  struct double_linked_list_element_s *prev;
  struct double_linked_list_element_s *next;
  int i;
  char *v;
} double_linked_list_element;

/**
 * The list itself
 */
typedef struct double_linked_list_s {
  double_linked_list_element *head;
} double_linked_list;

double_linked_list *make_list();
int size(double_linked_list *list);
void delete_list(double_linked_list *list);
int add_element(double_linked_list *list, int i, char *string);
int delete_element(double_linked_list *list, int i);
char *lookup_element(double_linked_list *list, int i);
void print_list(double_linked_list *list);

int main(int argc, char **argv) {
  double_linked_list *list = make_list();

  int i;
  for (i=1; i<argc; i++) {
    char *arg = argv[i];
    switch (arg[0]) {
      case 'a': {
        int i;
        char *v = (char*) malloc(strlen(arg));

        sscanf(arg, "a %d %[A-Z-a-z]", &i, v);

        int result = add_element(list, i, v);
        if (result == 1) printf("Adding %d overwrite old value.\n", i);
        }
        break;
      case 'r': {
        int i;

        sscanf(arg, "r %d", &i);

        int result = delete_element(list, i);
        if (result == 0) printf("Err: Element %d not found.\n", i);
        }
        break;
      case 'l': {
        int i;

        sscanf(arg, "l %d", &i);

        char *result = lookup_element(list, i);
        if (result == NULL) printf("Err: Element %d not found.\n", i);
        else printf("Element %d is '%s'\n", i, result);
        }
        break;
      default: {
        printf("Error: Unknown command '%c'.\n", arg[0]);
        return -1;
        }
     }

     print_list(list);
  }

  delete_list(list);

  return 0;
}

/**
 * This function should allocate a new list structure (via malloc)
 * and return it.  Later, this list should be delete-able via 
 * the delete_list function.
 */
double_linked_list *make_list() {
  double_linked_list *list = (double_linked_list *) malloc(sizeof(double_linked_list));
  list->head = NULL;
  return list;
}

/**
 * This function should delete a previously allocated list.  If
 * the list has any elements in it, the function should first
 * delete all the elements (freeing the memory associated with them)
 * before freeing the list itself.
 */
void delete_list(double_linked_list *list) {
  while (size(list) > 0) 
    delete_element(list, list->head->i);
  
  free(list);
}

/**
 * This function should return the number of elements present in
 * the list.
 */
int size(double_linked_list *list) {
  // TODO: Should return the number of elements in the list

  return 0; 
}

/**
 * This function should add the given element to the provided
 * list.  Note that you *must* create a copy of the string for
 * use in the list (i.e., the caller is allowed to delete/overwrite
 * the string after this function returns).  If the list already
 * has an element with the given int value, you should delete
 * that element and overwrite it with the new value.
 *
 * **You should store the elements in the list in numerical order 
 *   based on the ints, from lowest to highest.**
 *
 * You should return 0 if the element did not exist previously,
 * or 1 if the element did exist previously and you overwrite it.
 */
int times_called = 0;
int add_element(double_linked_list *list, int i, char *string) {
  // TODO: Fill in this function
  times_called++;
  int counter = 0;
  int out = 0;
  char *str = string;
  double_linked_list_element *node = (double_linked_list_element *) malloc(sizeof(double_linked_list_element));
  double_linked_list_element *temp = (double_linked_list_element *) malloc(sizeof(double_linked_list_element));
	node->prev = NULL;
	node->next = NULL; 
	node->i = i;
	node->v = str;
  printf("times called is %d\n", times_called);
  if(times_called == 1){
	list->head = node;
	return 0;
  } else if(list->head->i == node->i){
	list->head->v = node->v;
	return 1;
  } else if(list->head->i > node->i){
	*temp = *list->head;
	printf("temp i and v are %d, %s\n", temp->i, temp->v);
	list->head->i = node->i;
	printf("temp i and v are %d, %s\n", temp->i, temp->v);
	list->head->v = node->v;
	printf("temp i and v are %d, %s\n", temp->i, temp->v);
	list->head->next = node;
	node->i = temp->i;
	node->v = temp->v;
	return 0;
  } else {
	 while(list->head->i <= node->i){
		if(list->head->next != NULL){
			list->head->next = node;
			out = 0;
			break;
		} else if(list->head->next->i == node->i){
			list->head->next->v = node->v;
			out = 1;
			break;
		} else if(list->head->next->i < node->i){
			list->head->next->prev = node;
			node->next = list->head->next;
			node->prev = list->head->prev;
			list->head->next = node;
			out = 0;
			break;
		} else {
			list->head = list->head->next;
			counter++;
		}
	}
  }
  if(counter > 0){
	while(counter > 0){
		list->head = list->head->prev;
		counter--;
	}
  }
  return out;
}

/**
 * This function should delete the element in the list with 
 * the corresponding int value.  It should be sure to free() any
 * allocated memory.  It should return 0 if the element was not
 * found, and 1 if the element was found.
 */
int delete_element(double_linked_list *list, int i) {
  // TODO: Fill in this function

  return 0;
}

/**
 * This function should look up the given element in the list, and
 * shoudl return the correspoinding string value.  If the element is
 * found, it should return the list's copy of the string (e.g., the
 * caller is *not* expected to change or free() the string).  If
 * the element is not found, it should return NULL.
 */
char *lookup_element(double_linked_list *list, int i) {
  // TODO: Fill in this function

  return NULL;
}

/**
 * Prints out the list as a utility function. Do not modify this
 * function, but you're welcome to use it during debugging.
 */
void print_list(double_linked_list *list) {
  printf("[");

  double_linked_list_element *e = list->head;
  
  while (e != NULL) {
    printf(" {%d, '%s'}", e->i, e->v);
    e = e->next;
  }

  printf(" ]\n");
}
 

