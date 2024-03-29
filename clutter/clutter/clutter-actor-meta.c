/*
 * Clutter.
 *
 * An OpenGL based 'interactive canvas' library.
 *
 * Copyright (C) 2010  Intel Corporation.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author:
 *   Emmanuele Bassi <ebassi@linux.intel.com>
 */

/**
 * ClutterActorMeta:
 * 
 * Base class of actor modifiers
 *
 * #ClutterActorMeta is an abstract class providing a common API for
 * modifiers of [class@Actor] behaviour, appearance or layout.
 *
 * A #ClutterActorMeta can only be owned by a single [class@Actor] at
 * any time.
 *
 * Every sub-class of #ClutterActorMeta should check if the
 * [property@ActorMeta:enabled] property is set to %TRUE before applying
 * any kind of modification.
 */

#include "config.h"

#include "clutter/clutter-actor-meta-private.h"

#include "clutter/clutter-debug.h"
#include "clutter/clutter-private.h"

struct _ClutterActorMetaPrivate
{
  ClutterActor *actor;
  gulong destroy_id;

  gchar *name;

  guint is_enabled : 1;

  gint priority;
};

enum
{
  PROP_0,

  PROP_ACTOR,
  PROP_NAME,
  PROP_ENABLED,

  PROP_LAST
};

static GParamSpec *obj_props[PROP_LAST];

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (ClutterActorMeta,
                                     clutter_actor_meta,
                                     G_TYPE_INITIALLY_UNOWNED)

static void
on_actor_destroy (ClutterActor     *actor,
                  ClutterActorMeta *meta)
{
  ClutterActorMetaPrivate *priv =
    clutter_actor_meta_get_instance_private (meta);

  priv->actor = NULL;
}

static void
clutter_actor_meta_real_set_actor (ClutterActorMeta *meta,
                                   ClutterActor     *actor)
{
  ClutterActorMetaPrivate *priv =
    clutter_actor_meta_get_instance_private (meta);

  g_warn_if_fail (!priv->actor ||
                  !CLUTTER_ACTOR_IN_PAINT (priv->actor));
  g_warn_if_fail (!actor || !CLUTTER_ACTOR_IN_PAINT (actor));

  if (priv->actor == actor)
    return;

  g_clear_signal_handler (&priv->destroy_id, priv->actor);

  priv->actor = actor;

  if (priv->actor != NULL)
    priv->destroy_id = g_signal_connect (priv->actor, "destroy",
                                         G_CALLBACK (on_actor_destroy),
                                         meta);

  g_object_notify_by_pspec (G_OBJECT (meta), obj_props[PROP_ACTOR]);
}

static void
clutter_actor_meta_real_set_enabled (ClutterActorMeta *meta,
                                     gboolean          is_enabled)
{
  ClutterActorMetaPrivate *priv =
    clutter_actor_meta_get_instance_private (meta);

  g_warn_if_fail (!priv->actor ||
                  !CLUTTER_ACTOR_IN_PAINT (priv->actor));

  priv->is_enabled = is_enabled;

  g_object_notify_by_pspec (G_OBJECT (meta), obj_props[PROP_ENABLED]);
}

static void
clutter_actor_meta_set_property (GObject      *gobject,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  ClutterActorMeta *meta = CLUTTER_ACTOR_META (gobject);

  switch (prop_id)
    {
    case PROP_NAME:
      clutter_actor_meta_set_name (meta, g_value_get_string (value));
      break;

    case PROP_ENABLED:
      clutter_actor_meta_set_enabled (meta, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
clutter_actor_meta_get_property (GObject    *gobject,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  ClutterActorMetaPrivate *priv =
    clutter_actor_meta_get_instance_private (CLUTTER_ACTOR_META (gobject));

  switch (prop_id)
    {
    case PROP_ACTOR:
      g_value_set_object (value, priv->actor);
      break;

    case PROP_NAME:
      g_value_set_string (value, priv->name);
      break;

    case PROP_ENABLED:
      g_value_set_boolean (value, priv->is_enabled);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
clutter_actor_meta_finalize (GObject *gobject)
{
  ClutterActorMetaPrivate *priv =
    clutter_actor_meta_get_instance_private (CLUTTER_ACTOR_META (gobject));

  if (priv->actor != NULL)
    g_clear_signal_handler (&priv->destroy_id, priv->actor);

  g_free (priv->name);

  G_OBJECT_CLASS (clutter_actor_meta_parent_class)->finalize (gobject);
}

void
clutter_actor_meta_class_init (ClutterActorMetaClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  klass->set_actor = clutter_actor_meta_real_set_actor;
  klass->set_enabled = clutter_actor_meta_real_set_enabled;

  /**
   * ClutterActorMeta:actor:
   *
   * The #ClutterActor attached to the #ClutterActorMeta instance
   */
  obj_props[PROP_ACTOR] =
    g_param_spec_object ("actor", NULL, NULL,
                         CLUTTER_TYPE_ACTOR,
                         G_PARAM_READABLE |
                         G_PARAM_STATIC_STRINGS |
                         G_PARAM_EXPLICIT_NOTIFY);

  /**
   * ClutterActorMeta:name:
   *
   * The unique name to access the #ClutterActorMeta
   */
  obj_props[PROP_NAME] =
    g_param_spec_string ("name", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE |
                         G_PARAM_STATIC_STRINGS);

  /**
   * ClutterActorMeta:enabled:
   *
   * Whether or not the #ClutterActorMeta is enabled
   */
  obj_props[PROP_ENABLED] =
    g_param_spec_boolean ("enabled", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE |
                          G_PARAM_STATIC_STRINGS);

  gobject_class->finalize = clutter_actor_meta_finalize;
  gobject_class->set_property = clutter_actor_meta_set_property;
  gobject_class->get_property = clutter_actor_meta_get_property;
  g_object_class_install_properties (gobject_class,
                                     PROP_LAST,
                                     obj_props);
}

void
clutter_actor_meta_init (ClutterActorMeta *self)
{
  ClutterActorMetaPrivate *priv =
    clutter_actor_meta_get_instance_private (self);

  priv->is_enabled = TRUE;
  priv->priority = CLUTTER_ACTOR_META_PRIORITY_DEFAULT;
}

/**
 * clutter_actor_meta_set_name:
 * @meta: a #ClutterActorMeta
 * @name: the name of @meta
 *
 * Sets the name of @meta
 *
 * The name can be used to identify the #ClutterActorMeta instance
 */
void
clutter_actor_meta_set_name (ClutterActorMeta *meta,
                             const gchar      *name)
{
  ClutterActorMetaPrivate *priv;

  g_return_if_fail (CLUTTER_IS_ACTOR_META (meta));

  priv = clutter_actor_meta_get_instance_private (meta);

  if (g_strcmp0 (priv->name, name) == 0)
    return;

  g_free (priv->name);
  priv->name = g_strdup (name);

  g_object_notify_by_pspec (G_OBJECT (meta), obj_props[PROP_NAME]);
}

/**
 * clutter_actor_meta_get_name:
 * @meta: a #ClutterActorMeta
 *
 * Retrieves the name set using [method@ActorMeta.set_name]
 *
 * Return value: (transfer none): the name of the #ClutterActorMeta
 *   instance, or %NULL if none was set. The returned string is owned
 *   by the #ClutterActorMeta instance and it should not be modified
 *   or freed
 */
const gchar *
clutter_actor_meta_get_name (ClutterActorMeta *meta)
{
  ClutterActorMetaPrivate *priv;

  g_return_val_if_fail (CLUTTER_IS_ACTOR_META (meta), NULL);

  priv = clutter_actor_meta_get_instance_private (meta);

  return priv->name;
}

/**
 * clutter_actor_meta_set_enabled:
 * @meta: a #ClutterActorMeta
 * @is_enabled: whether @meta is enabled
 *
 * Sets whether @meta should be enabled or not
 */
void
clutter_actor_meta_set_enabled (ClutterActorMeta *meta,
                                gboolean          is_enabled)
{
  ClutterActorMetaPrivate *priv;

  g_return_if_fail (CLUTTER_IS_ACTOR_META (meta));

  priv = clutter_actor_meta_get_instance_private (meta);
  is_enabled = !!is_enabled;

  if (priv->is_enabled == is_enabled)
    return;

  CLUTTER_ACTOR_META_GET_CLASS (meta)->set_enabled (meta, is_enabled);
}

/**
 * clutter_actor_meta_get_enabled:
 * @meta: a #ClutterActorMeta
 *
 * Retrieves whether @meta is enabled
 *
 * Return value: %TRUE if the #ClutterActorMeta instance is enabled
 */
gboolean
clutter_actor_meta_get_enabled (ClutterActorMeta *meta)
{
  ClutterActorMetaPrivate *priv;

  g_return_val_if_fail (CLUTTER_IS_ACTOR_META (meta), FALSE);

  priv = clutter_actor_meta_get_instance_private (meta);

  return priv->is_enabled;
}

/*
 * _clutter_actor_meta_set_actor
 * @meta: a #ClutterActorMeta
 * @actor: a #ClutterActor or %NULL
 *
 * Sets or unsets a back pointer to the #ClutterActor that owns
 * the @meta
 */
static void
_clutter_actor_meta_set_actor (ClutterActorMeta *meta,
                               ClutterActor     *actor)
{
  g_return_if_fail (CLUTTER_IS_ACTOR_META (meta));
  g_return_if_fail (actor == NULL || CLUTTER_IS_ACTOR (actor));

  CLUTTER_ACTOR_META_GET_CLASS (meta)->set_actor (meta, actor);
}

/**
 * clutter_actor_meta_get_actor:
 * @meta: a #ClutterActorMeta
 *
 * Retrieves a pointer to the [class@Actor] that owns @meta
 *
 * Return value: (transfer none): a pointer to a #ClutterActor or %NULL
 */
ClutterActor *
clutter_actor_meta_get_actor (ClutterActorMeta *meta)
{
  ClutterActorMetaPrivate *priv;

  g_return_val_if_fail (CLUTTER_IS_ACTOR_META (meta), NULL);

  priv = clutter_actor_meta_get_instance_private (meta);

  return priv->actor;
}

void
_clutter_actor_meta_set_priority (ClutterActorMeta *meta,
                                  gint priority)
{
  ClutterActorMetaPrivate *priv;

  g_return_if_fail (CLUTTER_IS_ACTOR_META (meta));

  priv = clutter_actor_meta_get_instance_private (meta);

  /* This property shouldn't be modified after the actor meta is in
     use because ClutterMetaGroup doesn't resort the list when it
     changes. If we made the priority public then we could either make
     the priority a construct-only property or listen for
     notifications on the property from the ClutterMetaGroup and
     resort. */
  g_return_if_fail (priv->actor == NULL);

  priv->priority = priority;
}

static gint
_clutter_actor_meta_get_priority (ClutterActorMeta *meta)
{
  ClutterActorMetaPrivate *priv;

  g_return_val_if_fail (CLUTTER_IS_ACTOR_META (meta), 0);

  priv = clutter_actor_meta_get_instance_private (meta);

  return priv->priority;
}

static gboolean
_clutter_actor_meta_is_internal (ClutterActorMeta *meta)
{
  ClutterActorMetaPrivate *priv =
    clutter_actor_meta_get_instance_private (meta);
  gint priority = priv->priority;

  return (priority <= CLUTTER_ACTOR_META_PRIORITY_INTERNAL_LOW ||
          priority >= CLUTTER_ACTOR_META_PRIORITY_INTERNAL_HIGH);
}

/*
 * ClutterMetaGroup: a collection of ClutterActorMeta instances
 */

G_DEFINE_FINAL_TYPE (ClutterMetaGroup, _clutter_meta_group, G_TYPE_OBJECT);

static void
_clutter_meta_group_dispose (GObject *gobject)
{
  _clutter_meta_group_clear_metas (CLUTTER_META_GROUP (gobject));

  G_OBJECT_CLASS (_clutter_meta_group_parent_class)->dispose (gobject);
}

static void
_clutter_meta_group_class_init (ClutterMetaGroupClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->dispose = _clutter_meta_group_dispose;
}

static void
_clutter_meta_group_init (ClutterMetaGroup *self)
{
}

/*
 * _clutter_meta_group_add_meta:
 * @group: a #ClutterMetaGroup
 * @meta: a #ClutterActorMeta to add
 *
 * Adds @meta to @group
 *
 * This function will remove the floating reference of @meta or, if the
 * floating reference has already been sunk, add a reference to it
 */
void
_clutter_meta_group_add_meta (ClutterMetaGroup *group,
                              ClutterActorMeta *meta)
{
  ClutterActorMetaPrivate *priv =
    clutter_actor_meta_get_instance_private (meta);
  GList *prev = NULL, *l;

  if (priv->actor != NULL)
    {
      g_warning ("The meta of type '%s' with name '%s' is "
                 "already attached to actor '%s'",
                 G_OBJECT_TYPE_NAME (meta),
                 priv->name != NULL
                   ? priv->name
                   : "<unknown>",
                 clutter_actor_get_name (priv->actor) != NULL
                   ? clutter_actor_get_name (priv->actor)
                   : G_OBJECT_TYPE_NAME (priv->actor));
      return;
    }

  /* Find a meta that has lower priority and insert before that */
  for (l = group->meta; l; l = l->next)
    if (_clutter_actor_meta_get_priority (l->data) <
        _clutter_actor_meta_get_priority (meta))
      break;
    else
      prev = l;

  if (prev == NULL)
    group->meta = g_list_prepend (group->meta, meta);
  else
    {
      prev->next = g_list_prepend (prev->next, meta);
      prev->next->prev = prev;
    }

  g_object_ref_sink (meta);

  _clutter_actor_meta_set_actor (meta, group->actor);
}

/*
 * _clutter_meta_group_remove_meta:
 * @group: a #ClutterMetaGroup
 * @meta: a #ClutterActorMeta to remove
 *
 * Removes @meta from @group and releases the reference being held on it
 */
void
_clutter_meta_group_remove_meta (ClutterMetaGroup *group,
                                 ClutterActorMeta *meta)
{
  ClutterActorMetaPrivate *priv =
    clutter_actor_meta_get_instance_private (meta);

  if (priv->actor != group->actor)
    {
      g_warning ("The meta of type '%s' with name '%s' is not "
                 "attached to the actor '%s'",
                 G_OBJECT_TYPE_NAME (meta),
                 priv->name != NULL
                   ? priv->name
                   : "<unknown>",
                 clutter_actor_get_name (group->actor) != NULL
                   ? clutter_actor_get_name (group->actor)
                   : G_OBJECT_TYPE_NAME (group->actor));
      return;
    }

  _clutter_actor_meta_set_actor (meta, NULL);

  group->meta = g_list_remove (group->meta, meta);
  g_object_unref (meta);
}

/*
 * _clutter_meta_group_peek_metas:
 * @group: a #ClutterMetaGroup
 *
 * Returns a pointer to the #ClutterActorMeta list
 *
 * Return value: a const pointer to the #GList of #ClutterActorMeta
 */
const GList *
_clutter_meta_group_peek_metas (ClutterMetaGroup *group)
{
  return group->meta;
}

/*
 * _clutter_meta_group_get_metas_no_internal:
 * @group: a #ClutterMetaGroup
 *
 * Returns a new allocated list containing all of the metas that don't
 * have an internal priority.
 *
 * Return value: A GList containing non-internal metas. Free with
 * g_list_free.
 */
GList *
_clutter_meta_group_get_metas_no_internal (ClutterMetaGroup *group)
{
  GList *ret = NULL;
  GList *l;

  /* Build a new list filtering out the internal metas */
  for (l = group->meta; l; l = l->next)
    if (!_clutter_actor_meta_is_internal (l->data))
      ret = g_list_prepend (ret, l->data);

  return g_list_reverse (ret);
}

/*
 * _clutter_meta_group_has_metas_no_internal:
 * @group: a #ClutterMetaGroup
 *
 * Returns whether the group has any metas that don't have an internal priority.
 *
 * Return value: %TRUE if metas without internal priority exist
 *   %FALSE otherwise
 */
gboolean
_clutter_meta_group_has_metas_no_internal (ClutterMetaGroup *group)
{
  GList *l;

  for (l = group->meta; l; l = l->next)
    if (!_clutter_actor_meta_is_internal (l->data))
      return TRUE;

  return FALSE;
}

/*
 * _clutter_meta_group_clear_metas:
 * @group: a #ClutterMetaGroup
 *
 * Clears @group of all #ClutterActorMeta instances and releases
 * the reference on them
 */
void
_clutter_meta_group_clear_metas (ClutterMetaGroup *group)
{
  g_list_foreach (group->meta, (GFunc) _clutter_actor_meta_set_actor, NULL);

  g_list_free_full (group->meta, g_object_unref);
  group->meta = NULL;
}

/*
 * _clutter_meta_group_clear_metas_no_internal:
 * @group: a #ClutterMetaGroup
 *
 * Clears @group of all #ClutterActorMeta instances that don't have an
 * internal priority and releases the reference on them
 */
void
_clutter_meta_group_clear_metas_no_internal (ClutterMetaGroup *group)
{
  GList *internal_list = NULL;
  GList *l, *next;

  for (l = group->meta; l; l = next)
    {
      next = l->next;

      if (_clutter_actor_meta_is_internal (l->data))
        {
          if (internal_list)
            internal_list->prev = l;
          l->next = internal_list;
          l->prev = NULL;
          internal_list = l;
        }
      else
        {
          _clutter_actor_meta_set_actor (l->data, NULL);
          g_object_unref (l->data);
          g_list_free_1 (l);
        }
    }

  group->meta = g_list_reverse (internal_list);
}

/*
 * _clutter_meta_group_get_meta:
 * @group: a #ClutterMetaGroup
 * @name: the name of the #ClutterActorMeta to retrieve
 *
 * Retrieves a named #ClutterActorMeta from @group
 *
 * Return value: a #ClutterActorMeta for the given name, or %NULL
 */
ClutterActorMeta *
_clutter_meta_group_get_meta (ClutterMetaGroup *group,
                              const gchar      *name)
{
  GList *l;

  for (l = group->meta; l != NULL; l = l->next)
    {
      ClutterActorMeta *meta = l->data;
      ClutterActorMetaPrivate *priv =
        clutter_actor_meta_get_instance_private (meta);

      if (g_strcmp0 (priv->name, name) == 0)
        return meta;
    }

  return NULL;
}

/*< private >
 * clutter_actor_meta_get_debug_name:
 * @meta: a #ClutterActorMeta
 *
 * Retrieves the name of the @meta for debugging purposes.
 *
 * Return value: (transfer none): the name of the @meta. The returned
 *   string is owned by the @meta instance and it should not be
 *   modified or freed
 */
const gchar *
_clutter_actor_meta_get_debug_name (ClutterActorMeta *meta)
{
  ClutterActorMetaPrivate *priv =
    clutter_actor_meta_get_instance_private (meta);

  return priv->name != NULL ? priv->name : G_OBJECT_TYPE_NAME (meta);
}
