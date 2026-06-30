#include "internal.h"

rv_status rv_param_list_push(rv_param_list *list, rv_param *param)
{
    rv_param *p;
    size_t cap;
    if (!list || !param) return RV_ERR_ARGUMENT;
    if (list->len == list->cap) {
        cap = list->cap ? list->cap * 2 : 2;
        p = (rv_param *)realloc(list->items, cap * sizeof(*p));
        if (!p) return RV_ERR_NOMEM;
        list->items = p;
        list->cap = cap;
    }
    list->items[list->len++] = *param;
    memset(param, 0, sizeof(*param));
    return RV_OK;
}

void rv_param_list_free(rv_param_list *list)
{
    size_t i;
    if (!list) return;
    for (i = 0; i < list->len; i++) {
        free(list->items[i].name);
        free(list->items[i].value);
    }
    free(list->items);
    list->items = NULL;
    list->len = 0;
    list->cap = 0;
}

void rv_content_type_free(rv_content_type *ct)
{
    if (!ct) return;
    free(ct->media_type);
    free(ct->subtype);
    rv_param_list_free(&ct->params);
    memset(ct, 0, sizeof(*ct));
}

void rv_header_free(rv_header *hdr)
{
    if (!hdr) return;
    free(hdr->name);
    free(hdr->value);
    free(hdr->raw);
    rv_content_type_free(&hdr->content_type);
    memset(hdr, 0, sizeof(*hdr));
}

rv_status rv_header_list_push(rv_header_list *list, rv_header *hdr)
{
    rv_header *p;
    size_t cap;
    if (!list || !hdr) return RV_ERR_ARGUMENT;
    if (list->len == list->cap) {
        cap = list->cap ? list->cap * 2 : 16;
        p = (rv_header *)realloc(list->items, cap * sizeof(*p));
        if (!p) return RV_ERR_NOMEM;
        list->items = p;
        list->cap = cap;
    }
    list->items[list->len++] = *hdr;
    memset(hdr, 0, sizeof(*hdr));
    return RV_OK;
}

void rv_header_list_free(rv_header_list *list)
{
    size_t i;
    if (!list) return;
    for (i = 0; i < list->len; i++) {
        rv_header_free(&list->items[i]);
    }
    free(list->items);
    list->items = NULL;
    list->len = 0;
    list->cap = 0;
}

void rv_message_init(rv_message *msg)
{
    if (!msg) return;
    memset(msg, 0, sizeof(*msg));
}

void rv_part_free(rv_part *part)
{
    size_t i;
    if (!part) return;
    rv_header_list_free(&part->headers);
    free(part->body);
    free(part->boundary);
    free(part->preamble);
    free(part->epilogue);
    free(part->epilogue_note_owned);
    for (i = 0; i < part->child_count; i++) {
        rv_part_free(&part->children[i]);
    }
    free(part->children);
    memset(part, 0, sizeof(*part));
}

void rv_message_free(rv_message *msg)
{
    if (!msg) return;
    rv_header_list_free(&msg->headers);
    rv_part_free(&msg->root);
    free(msg->body);
    free(msg->normalized_id);
    memset(msg, 0, sizeof(*msg));
}

rv_status rv_mbox_entry_push(rv_mbox *box, rv_mbox_entry *entry)
{
    rv_mbox_entry *p;
    size_t cap;
    if (!box || !entry) return RV_ERR_ARGUMENT;
    if (box->len == box->cap) {
        cap = box->cap ? box->cap * 2 : 8;
        p = (rv_mbox_entry *)realloc(box->entries, cap * sizeof(*p));
        if (!p) return RV_ERR_NOMEM;
        box->entries = p;
        box->cap = cap;
    }
    box->entries[box->len++] = *entry;
    memset(entry, 0, sizeof(*entry));
    return RV_OK;
}

void rv_mbox_free(rv_mbox *box)
{
    size_t i;
    if (!box) return;
    for (i = 0; i < box->len; i++) {
        free(box->entries[i].sender);
        free(box->entries[i].date_text);
        rv_message_free(&box->entries[i].message);
    }
    free(box->entries);
    box->entries = NULL;
    box->len = 0;
    box->cap = 0;
}

rv_header *rv_message_find_header(rv_message *msg, const char *name)
{
    size_t i;
    if (!msg || !name) return NULL;
    for (i = 0; i < msg->headers.len; i++) {
        if (rv_header_name_equal(msg->headers.items[i].name, name)) return &msg->headers.items[i];
    }
    return NULL;
}

const rv_header *rv_message_find_header_const(const rv_message *msg, const char *name)
{
    size_t i;
    if (!msg || !name) return NULL;
    for (i = 0; i < msg->headers.len; i++) {
        if (rv_header_name_equal(msg->headers.items[i].name, name)) return &msg->headers.items[i];
    }
    return NULL;
}
