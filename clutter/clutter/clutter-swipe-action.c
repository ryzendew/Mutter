/*
 * Clutter.
 *
 * An OpenGL based 'interactive canvas' library.
 *
 * Copyright (C) 2010  Intel Corporation.
 * Copyright (C) 2011  Robert Bosch Car Multimedia GmbH.
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
 *   Tomeu Vizoso <tomeu.vizoso@collabora.co.uk>
 *
 * Based on ClutterDragAction, written by:
 *   Emmanuele Bassi <ebassi@linux.intel.com>
 */

/**
 * ClutterSwipeAction:
 * 
 * Action for swipe gestures
 *
 * #ClutterSwipeAction is a sub-class of [class@GestureAction] that implements
 * the logic for recognizing swipe gestures.
 */

#include "config.h"

#include "clutter/clutter-swipe-action.h"

#include "clutter/clutter-debug.h"
#include "clutter/clutter-enum-types.h"
#include "clutter/clutter-marshal.h"
#include "clutter/clutter-private.h"

typedef struct _ClutterSwipeActionPrivate
{
  ClutterSwipeDirection h_direction;
  ClutterSwipeDirection v_direction;

  float distance_x, distance_y;
} ClutterSwipeActionPrivate;

enum
{
  SWIPE,

  LAST_SIGNAL
};

static guint swipe_signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE_WITH_PRIVATE (ClutterSwipeAction, clutter_swipe_action, CLUTTER_TYPE_GESTURE_ACTION)

static gboolean
gesture_begin (ClutterGestureAction  *action,
               ClutterActor          *actor)
{
  ClutterSwipeActionPrivate *priv =
    clutter_swipe_action_get_instance_private (CLUTTER_SWIPE_ACTION (action));

  /* reset the state at the beginning of a new gesture */
  priv->h_direction = 0;
  priv->v_direction = 0;

  g_object_get (action,
                "threshold-trigger-distance-x", &priv->distance_x,
                "threshold-trigger-distance-y", &priv->distance_y,
                NULL);

  return TRUE;
}

static gboolean
gesture_progress (ClutterGestureAction *action,
                  ClutterActor         *actor)
{
  ClutterSwipeActionPrivate *priv =
    clutter_swipe_action_get_instance_private (CLUTTER_SWIPE_ACTION (action));
  gfloat press_x, press_y;
  gfloat motion_x, motion_y;
  gfloat delta_x, delta_y;
  ClutterSwipeDirection h_direction = 0, v_direction = 0;

  clutter_gesture_action_get_press_coords (action,
                                           0,
                                           &press_x,
                                           &press_y);

  clutter_gesture_action_get_motion_coords (action,
                                            0,
                                            &motion_x,
                                            &motion_y);

  delta_x = press_x - motion_x;
  delta_y = press_y - motion_y;

  if (delta_x >= priv->distance_x)
    h_direction = CLUTTER_SWIPE_DIRECTION_RIGHT;
  else if (delta_x < -priv->distance_x)
    h_direction = CLUTTER_SWIPE_DIRECTION_LEFT;

  if (delta_y >= priv->distance_y)
    v_direction = CLUTTER_SWIPE_DIRECTION_DOWN;
  else if (delta_y < -priv->distance_y)
    v_direction = CLUTTER_SWIPE_DIRECTION_UP;

  /* cancel gesture on direction reversal */
  if (priv->h_direction == 0)
    priv->h_direction = h_direction;

  if (priv->v_direction == 0)
    priv->v_direction = v_direction;

  if (priv->h_direction != h_direction)
    return FALSE;

  if (priv->v_direction != v_direction)
    return FALSE;

  return TRUE;
}

static void
gesture_end (ClutterGestureAction *action,
             ClutterActor         *actor)
{
  ClutterSwipeActionPrivate *priv =
    clutter_swipe_action_get_instance_private (CLUTTER_SWIPE_ACTION (action));
  gfloat press_x, press_y;
  gfloat release_x, release_y;
  ClutterSwipeDirection direction = 0;
  const ClutterEvent *last_event;

  clutter_gesture_action_get_press_coords (action,
                                           0,
                                           &press_x, &press_y);

  /* Check the last event instead of get_release_coords(), this
   * might not be the sequence that finished on multi-finger swipes.
   */
  last_event = clutter_gesture_action_get_last_event (action, 0);
  clutter_event_get_coords (last_event, &release_x, &release_y);

  if (release_x - press_x > priv->distance_x)
    direction |= CLUTTER_SWIPE_DIRECTION_RIGHT;
  else if (press_x - release_x > priv->distance_x)
    direction |= CLUTTER_SWIPE_DIRECTION_LEFT;

  if (release_y - press_y > priv->distance_y)
    direction |= CLUTTER_SWIPE_DIRECTION_DOWN;
  else if (press_y - release_y > priv->distance_y)
    direction |= CLUTTER_SWIPE_DIRECTION_UP;

  g_signal_emit (action, swipe_signals[SWIPE], 0, actor, direction);
}

static void
clutter_swipe_action_constructed (GObject *object)
{
  clutter_gesture_action_set_threshold_trigger_edge (CLUTTER_GESTURE_ACTION (object),
                                                     CLUTTER_GESTURE_TRIGGER_EDGE_AFTER);
}

static void
clutter_swipe_action_class_init (ClutterSwipeActionClass *klass)
{
  ClutterGestureActionClass *gesture_class =
      CLUTTER_GESTURE_ACTION_CLASS (klass);
  GObjectClass *object_class =
      G_OBJECT_CLASS (klass);

  object_class->constructed = clutter_swipe_action_constructed;

  gesture_class->gesture_begin = gesture_begin;
  gesture_class->gesture_progress = gesture_progress;
  gesture_class->gesture_end = gesture_end;

  /**
   * ClutterSwipeAction::swipe:
   * @action: the #ClutterSwipeAction that emitted the signal
   * @actor: the #ClutterActor attached to the @action
   * @direction: the main direction of the swipe gesture
   *
   * The signal is emitted when a swipe gesture is recognized on the
   * attached actor.
   *
   * Return value: %TRUE if the swipe should continue, and %FALSE if
   *   the swipe should be cancelled.
   */
  swipe_signals[SWIPE] =
    g_signal_new (I_("swipe"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, g_signal_accumulator_true_handled, NULL,
                  _clutter_marshal_BOOLEAN__OBJECT_FLAGS,
                  G_TYPE_BOOLEAN, 2,
                  CLUTTER_TYPE_ACTOR,
                  CLUTTER_TYPE_SWIPE_DIRECTION);
}

static void
clutter_swipe_action_init (ClutterSwipeAction *self)
{
}

/**
 * clutter_swipe_action_new:
 *
 * Creates a new #ClutterSwipeAction instance
 *
 * Return value: the newly created #ClutterSwipeAction
 */
ClutterAction *
clutter_swipe_action_new (void)
{
  return g_object_new (CLUTTER_TYPE_SWIPE_ACTION, NULL);
}
