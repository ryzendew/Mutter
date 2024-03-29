/*
 * Copyright (C) 2019 Red Hat
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <glib.h>

#include "backends/native/meta-kms-types.h"

typedef struct _MetaKmsPageFlipData MetaKmsPageFlipData;

MetaKmsPageFlipData * meta_kms_page_flip_data_new (MetaKmsImplDevice *impl_device,
                                                   MetaKmsCrtc       *crtc);

MetaKmsPageFlipData * meta_kms_page_flip_data_ref (MetaKmsPageFlipData *page_flip_data);

void meta_kms_page_flip_data_unref (MetaKmsPageFlipData *page_flip_data);

void meta_kms_page_flip_data_add_listener (MetaKmsPageFlipData                 *page_flip_data,
                                           const MetaKmsPageFlipListenerVtable *vtable,
                                           GMainContext                        *main_context,
                                           gpointer                             user_data,
                                           GDestroyNotify                       destroy_notify);

MetaKmsImplDevice * meta_kms_page_flip_data_get_impl_device (MetaKmsPageFlipData *page_flip_data);

MetaKmsCrtc * meta_kms_page_flip_data_get_crtc (MetaKmsPageFlipData *page_flip_data);

void meta_kms_page_flip_data_set_timings_in_impl (MetaKmsPageFlipData *page_flip_data,
                                                  unsigned int         sequence,
                                                  unsigned int         sec,
                                                  unsigned int         usec);

void meta_kms_page_flip_data_flipped_in_impl (MetaKmsPageFlipData *page_flip_data);

void meta_kms_page_flip_data_mode_set_fallback_in_impl (MetaKmsPageFlipData *page_flip_data);

void meta_kms_page_flip_data_discard_in_impl (MetaKmsPageFlipData *page_flip_data,
                                              const GError        *error);

void meta_kms_page_flip_data_make_symbolic (MetaKmsPageFlipData *page_flip_data);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (MetaKmsPageFlipData, meta_kms_page_flip_data_unref)
