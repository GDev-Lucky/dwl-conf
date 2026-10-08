/* SPLIT LAYOUT */

#ifndef DWL_LAYOUT_H
#define DWL_LAYOUT_H

typedef enum 
{
    SPLIT_LEFT_RIGHT,
    SPLIT_TOP_BOTTOM,
}
SplitDirection;

typedef enum
{
    FRAME_LEFT,
    FRAME_DOWN,
    FRAME_UP,
    FRAME_RIGHT,
}
FrameDirection;


struct TileNode {
    TileNode *parent;
    TileNode *first;
    TileNode *second;

    Client *client;

    float ratio;
    int split;

    Client *last_neighbor[4];
};

/* HELPERS */

static int
frame_visible(TileNode *node, Monitor *m)
{
    if (!node) {
        return 0;
    }

    if (node->client) {
        Client *c = node->client;

        return VISIBLEON(c, m)
            && !c->isfloating
            && !c->isfullscreen;
    }

    return frame_visible(node->first, m) || frame_visible(node->second, m);
}

static TileNode *
frame_first_leaf(TileNode *node)
{
    TileNode *leaf;

    if (!node) {
        return NULL;
    }

    if (node->client) {
        return node;
    }

    leaf = frame_first_leaf(node->first);

    if (leaf) {
        return leaf;
    }

    return frame_first_leaf(node->second);
}

static void
frame_insert(Client *c)
{
    Monitor *m = c->mon;
    TileNode *target = NULL;

    Client *w;
    TileNode *parent;
    TileNode *branch;
    TileNode *leaf;

    if (!m || c->tile) {
        return;
    }

    leaf = ecalloc(1, sizeof(*leaf));

    leaf->client = c;
    leaf->ratio = 0.5f;

    c->tile = leaf;

    /* FIRST WINDOW ON THE MONITOR */
    if (!m->tile_root) {
        m->tile_root = leaf;
        return;
    }

    /* FIND THE PREVIOUSLY FOCUSED VISIBLE TITLED FRAME */
    wl_list_for_each(w, &fstack, flink) {

        if (w == c) {
            continue;
        }

        if (w->mon != m || !w->tile) {
            continue;
        }

        if (!VISIBLEON(w, m)) {
            continue;
        }

        if (w->isfloating || w->isfullscreen) {
            continue;
        }

        target = w->tile;
        break;
    } 
    
    /* If NO TARGET USE EXSISTING LEAF */
    if (!target) {
        target = frame_first_leaf(m->tile_root);
    }
    
    /* SPLIT A INTO A AND B */
    parent = target->parent;
    branch = ecalloc(1, sizeof(*branch));

    branch->parent = parent;
    branch->first = target;
    branch->second = leaf;

    branch->ratio = 0.5f;
    branch->split = m->next_split;

    target->parent = branch;
    leaf->parent = branch;

    if (!parent) {
        m->tile_root = branch;
        return;
    }

    if (parent->first == target) {
        parent->first = branch;
    } 
    else {
        parent->second = branch;
    }

}

static void
frame_remove(Client *c)
{
    Monitor *m;
    TileNode *leaf = c->tile;
    TileNode *parent;
    TileNode *sibling;
    TileNode *grand;

    if (!leaf) {
        return;
    }

    m = c->mon;
    parent = leaf->parent;

    /* LAST WINDOW */
    if (!parent) {
        if (m) {
            m->tile_root = NULL;
        }
        c->tile = NULL;
        free(leaf);
        return;
    }

    if (parent->first == leaf) {
        sibling = parent->second;
    }
    else {
        sibling = parent->first;
    }

    grand = parent->parent;
    sibling->parent = grand;

    if (!grand)
    {
        if (m) 
        {
            m->tile_root = sibling;
        }
    }
    else if (grand->first == parent)
    {
        grand->first = sibling;
    }
    else 
    {
        grand->second = sibling;
    }

    c->tile = NULL;
    free(leaf);
    free(parent);
}

static void frame_arrange_node(TileNode *node, Monitor *m, struct wlr_box box)
{
    int first_visible;
    int second_visible;
    struct wlr_box first;
    struct wlr_box second;

    if (!node)
    {
        return;
    }

    if (node->client)
    {
        if (frame_visible(node, m))
        {
            resize(node->client, box, 0);
        }

        return;
    }
    
    first_visible = frame_visible(node->first, m);
    second_visible = frame_visible(node->second, m);

    if (!first_visible && !second_visible)
    {
        return;
    }

    if (!first_visible)
    {
        frame_arrange_node(node->second, m, box);
        return;
    }

    if (!second_visible)
    {
        frame_arrange_node(node->first, m, box);
        return;
    }
    
    first = box;
    second = box;

    if (node->split == SPLIT_LEFT_RIGHT)
    {
        int size = (int) (box.width * node->ratio);

        first.width = size;

        second.x += size;
        second.width -= size;
    }
    else
    {
        int size = (int) (box.height * node->ratio);

        first.height = size;

        second.y += size;
        second.height -= size;
    }

    frame_arrange_node(node->first, m, first);
    frame_arrange_node(node->second, m, second);
}

static void frame_layout(Monitor *m)
{
    if (!m || !m->tile_root)
    {
        return;
    }

    frame_arrange_node(m->tile_root, m, m->w);
}


static void frame_set_split(const Arg *arg)
{
    if (!selmon)
    {
        return;
    }

    selmon->next_split = arg->i;
}

static int frame_overlap(int a1, int a2, int b1, int b2)
{
    int start = MAX(a1, b1);
    int end = MIN(a2, b2);

    return end > start ? end - start : 0;
}

static int frame_opposite(int dir)
{
    switch(dir)
    {
        case FRAME_LEFT:
            return FRAME_RIGHT;

        case FRAME_RIGHT:
            return FRAME_LEFT;

        case FRAME_UP:
            return FRAME_DOWN;

        case FRAME_DOWN:
            return FRAME_UP;
    }

    return dir;
}

static int frame_direction_gap(Client *from, Client *to, int dir, int *gap, int *overlap)
{
    int fx1 = from->geom.x;
    int fy1 = from->geom.y;
    int fx2 = fx1 + from->geom.width;
    int fy2 = fy1 + from->geom.height;

    int tx1 = to->geom.x;
    int ty1 = to->geom.y;
    int tx2 = tx1 + to->geom.width;
    int ty2 = ty1 + to->geom.height;

    switch (dir)
    {
        case FRAME_LEFT:

            if (tx2 > fx1)
            {
                return 0;
            }

            *gap = fx1 - tx2;
            *overlap = frame_overlap(fy1, fy2, ty1, ty2);
            break;

        case FRAME_RIGHT:

            if (tx1 > fx2)
            {
                return 0;
            }

            *gap = tx1 - fx2;
            *overlap = frame_overlap(fy1, fy2, ty1, ty2);
            break;

        case FRAME_UP:

            if (ty2 > fy1)
            {
                return 0;
            }

            *gap = fy1 - ty2;
            *overlap = frame_overlap(fx1, fx2, tx1, tx2);
            break;

        case FRAME_DOWN:

            if (ty1 > fy2)
            {
                return 0;
            }

            *gap = ty1 - fy2;
            *overlap = frame_overlap(fx1, fx2, tx1, tx2);
            break;

        default:
            return 0;

    }

    return *overlap > 0;

}

static int frame_min_gap(Client *sel, int dir)
{
    Client* c;
    int best = -1;

    wl_list_for_each(c, &clients, link)
    {
        int gap;
        int overlap;

        if (c == sel)
        {
            continue;
        }

        if (c->mon != selmon)
        {
            continue;
        }

        if (!VISIBLEON(c, selmon))
        {
            continue;
        }

        if (c->isfloating || c->isfullscreen)
        {
            continue;
        }

        if (!frame_direction_gap(sel, c, dir, &gap, &overlap))
        {
            continue;
        }

        if (best < 0 || gap < best)
        {
            best = gap;
        }
    }

    return best;
}

static void frame_focus(const Arg *arg)
{
    Client *sel = focustop(selmon);
    Client *c;
    Client *best = NULL;
    Client *remembered;
    
    int dir = arg->i;
    int opposite;
    int min_gap;
    int gap;
    int overlap;

    if (!sel || !sel->tile || sel->isfullscreen)
    {
        return;
    }
    
    opposite = frame_opposite(dir);
    min_gap = frame_min_gap(sel, arg->i);
    
    if (min_gap >= 0)
    {
        remembered = sel->tile->last_neighbor[dir];

        if (
                remembered
                && remembered != sel
                && remembered->tile
                && remembered->mon == selmon
                && VISIBLEON(remembered, selmon)
                && !remembered->isfloating
                && frame_direction_gap(sel, remembered, dir, &gap, &overlap)
                && gap == min_gap
           )
        {
            best = remembered;
        }

        if (!best)
        {
            wl_list_for_each(c, &fstack, flink)
            {
                if (c == sel)
                {
                    continue;
                }

                if (c->mon != selmon)
                {
                    continue;
                }

                if (!VISIBLEON(c, selmon))
                {
                    continue;
                }

                if (c->isfloating || c->isfullscreen)
                {
                    continue;
                }

                if (!frame_direction_gap(sel, c, dir, &gap, &overlap))
                {
                    continue;
                }

                if (gap != min_gap)
                {
                    continue;
                }

                best = c;
                break;
            }
        }
    }

    if (!best)
    {
        return;
    }

    sel->tile->last_neighbor[dir] = best;

    if (best->tile)
    {
        best->tile->last_neighbor[opposite] = sel;
    }

    focusclient(best, 1);
}

    
static void frame_resize(const Arg *arg)
{
   Client *sel = focustop(selmon);
   TileNode *node;
   TileNode *parent;
   int wanted_split;
   float delta;

   if (!sel || !sel->tile || sel->isfloating || sel->isfullscreen)
   {
       return;
   }

   node = sel->tile;

   if (arg->i == FRAME_LEFT || arg->i == FRAME_RIGHT)
   {
       wanted_split = SPLIT_LEFT_RIGHT;
   }
   else
   {
       wanted_split = SPLIT_TOP_BOTTOM;
   }

   while (node->parent)
   {
       if (node->parent->split == wanted_split)
       {
           break;
       }

       node = node->parent;
   }

   if (!node->parent)
   {
       return;
   }
   parent = node->parent;

    
   if (arg->i == FRAME_LEFT || arg->i == FRAME_DOWN)
   {
       delta = 0.05f;
   }
   else
   {
       delta = -0.05f;
   }

   if (parent->second == node)
   {
       delta = -delta;
   }

   parent->ratio += delta;

   if (parent->ratio < 0.1f)
   {
       parent->ratio = 0.1f;
   }
   
   if (parent->ratio > 0.9f)
   {
       parent->ratio = 0.9f;
   }

   arrange(selmon);
}






#endif


