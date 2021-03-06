#include "strm.h"
#include <assert.h>

/* list is a polymorphic data structure which is either:
   - array of strm_value (struct strm_array)
   - linked list whose car is strm_value, and cdr is list or array or nil
   all functions that take struct strm_list* as arguments, should work for
   casted struct strm_array* as well.
*/

struct strm_list*
strm_list_new(const strm_value car, struct strm_list *cdr)
{
  struct strm_list *list;

  list = malloc(sizeof(struct strm_list));
  list->car = car;
  list->cdr = cdr;
  if (cdr) {
    list->len = cdr->len + 1;
  }
  else {
    list->len = 1;
  }

  return list;
}

static int
list_ary_eq(struct strm_list *a, struct strm_array *b)
{
  size_t i, j, len;

  for (i=0, len=b->len; a && i<len; i++) {
    if (!strm_value_eq(a->car, b->ptr[i])) return FALSE;
    a = a->cdr;
    if (a && a->type == STRM_OBJ_ARRAY) {
      for (j=0; i<len; i++, j++) {
        struct strm_array *a2 = (struct strm_array*)a;
        if (!strm_value_eq(a2->ptr[j], b->ptr[i])) return FALSE;
      }
      return TRUE;
    }
  }
  return i==len;
}

int
strm_list_eq(struct strm_list *a, struct strm_list *b)
{
  if (a == b) return TRUE;
  if (a->len != b->len) return FALSE;

  for (;;) {
    if (a->type == STRM_OBJ_ARRAY) {
      if (b->type == STRM_OBJ_ARRAY) {
        return strm_ary_eq((struct strm_array*)a, (struct strm_array*)b);
      }
      return list_ary_eq(b, (struct strm_array*)a);
    }
    if (b->type == STRM_OBJ_ARRAY) {
      return list_ary_eq(a, (struct strm_array*)b);
    }
    if (!strm_value_eq(a->car, b->car)) return FALSE;
    a = a->cdr; if (!a) return FALSE;
    b = b->cdr; if (!b) return FALSE;
  }
  return TRUE;
}

strm_value
strm_list_nth(struct strm_list *list, size_t n)
{
  size_t len = list->len;

  if (n > len) return strm_null_value();
  for (;;) {
    if (n == 0) return list->car;
    n--;
    list = list->cdr;
    if (list->type == STRM_OBJ_ARRAY) {
      struct strm_array *ary = (struct strm_array*)list;
      assert(ary->len > n);
      return ary->ptr[n];
    }
  }
}

struct strm_list*
strm_list_cons(strm_value car, strm_value cdr)
{
  if (cdr.type == STRM_VALUE_PTR) {
    if (!cdr.val.p) {
      return strm_list_new(car, NULL);
    }
    else {
      struct strm_object *obj = cdr.val.p;
      enum strm_obj_type type = obj->type;
      if (type == STRM_OBJ_LIST || type == STRM_OBJ_ARRAY) {
        return strm_list_new(car, strm_value_list(cdr));
      }
    }
  }

  {
    strm_value buf[2];

    buf[0] = car;
    buf[1] = cdr;
    return (struct strm_list*)strm_ary_new(buf, 2);
  }
}
