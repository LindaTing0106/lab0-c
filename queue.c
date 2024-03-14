#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *newnode = malloc(sizeof(struct list_head));
    if (!newnode)
        return NULL;
    INIT_LIST_HEAD(newnode);
    return newnode;
}

/* Free all storage used by queue */
void q_free(struct list_head *head) 
{
    if (!head)
        return;
    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, list){
        q_release_element(entry);
    }
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *newnode = malloc(sizeof(element_t));
    if (!newnode)
        return false;
    newnode -> value = malloc((strlen(s) + 1) *sizeof(char));
    if (!newnode->value){
        free(newnode);
        return false;
    }
    strcpy(newnode -> value,s);
    list_add(&newnode->list,head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *newnode = malloc(sizeof(element_t));
     if (!newnode)
        return false;
    newnode -> value = malloc((strlen(s) + 1) *sizeof(char));
    if (!newnode->value){
        free(newnode);
        return false;
    }
    strcpy(newnode -> value,s);
    list_add_tail(&newnode->list,head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    
    element_t *remove = list_first_entry(head, element_t, list);
    if (strlen (remove->value)>bufsize){
        strncpy(sp, remove->value,bufsize);
        *(sp+bufsize-1) = '\0';
    }
    else
        strncpy(sp, remove->value,strlen (remove->value)+1);
    list_del(&(remove->list));
    return remove;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *remove = list_last_entry(head, element_t, list);
    if (strlen (remove->value)>bufsize){
        strncpy(sp, remove->value,bufsize);
        *(sp+bufsize-1) = '\0';
    }
    else
        strncpy(sp, remove->value,strlen (remove->value)+1);
    list_del(&(remove->list));
    return remove;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head) return 0;
    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;
    int num = q_size(head);
    num = num/2 +1;
    int count = 0;
    struct list_head *del = head;
    while(count != num)
    { 
        del = del->next;
        count++;
    }   
    element_t *remove = list_entry(del, element_t, list);
    list_del(del);
    free(remove->value);
    free(remove);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head || list_empty(head))
        return false;
    struct list_head *find = head;
    bool del = 0;
    while(find->next != head){
        struct list_head *findsame = find->next;
        while(findsame->next != head&&find->next != head){
            element_t * findele = list_entry(find->next, element_t, list);
            element_t * sameele = list_entry(findsame->next, element_t, list);
            if (strcmp(findele->value,sameele->value)==0){
                del = 1;
                list_del(findsame->next);
                free(sameele->value);
                free(sameele);
            }
            else 
                findsame = findsame->next;
        }
        if (del){
            element_t * findele = list_entry(find->next, element_t, list);
            del = 0;
            list_del(find->next);
            free(findele->value);
            free(findele);
        }
        else 
            find = find->next;
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head)
        return;
    struct list_head *node;
    list_for_each (node, head) {
        if (node->next == head)
            break;
        list_move(node, node->next);
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head) 
{
    struct list_head *cur = head;
    struct list_head *pre = head->prev;
    struct list_head *nex = head->next;
  
    while(nex!=pre)
    {
        cur->prev = nex;
        cur->next = pre;
        pre = cur;
        cur = nex;
        nex = nex->next;
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    struct list_head *cur = head;
    struct list_head *seark = cur->next;
    
    struct list_head buf;
    INIT_LIST_HEAD(&buf);
    
    int count = 1;
       
    while (seark!=head){
        while (count!=k){
            if (seark == head)
                break;
            seark = seark->next;
            count ++;
        }
        if (seark == head)
            break;
        list_cut_position(&buf, cur, seark);
        q_reverse(&buf);
        seark = buf.prev;
        list_splice(&buf,cur);
        INIT_LIST_HEAD(&buf);
   
        cur = seark;
        seark = seark->next;
        count = 1;
    }
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend) {}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}