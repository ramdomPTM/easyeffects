/*
 *  Copyright © 2017-2025 Wellington Wallace
 *
 *  This file is part of Easy Effects.
 *
 *  Easy Effects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Easy Effects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <adwaita.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <glib/gvariant.h>
#include <glib/gvarianttype.h>
#include <gtk/deprecated/gtkcomboboxtext.h>
#include <gtk/gtk.h>
#include <gtk/gtkdropdown.h>
#include <gtk/gtklabel.h>
#include <gtk/gtklevelbar.h>
#include <gtk/gtkscale.h>
#include <gtk/gtkshortcut.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtkstringlist.h>
#include <gtk/gtkswitch.h>
#include <gtk/gtktogglebutton.h>
#include <sys/types.h>
#include <locale>
#define FMT_HEADER_ONLY
#include <fmt/core.h>
#include <fmt/format.h>
#include <glib/gi18n.h>
#include <string>
#include "string_literal_wrapper.hpp"
#include "util.hpp"

namespace ui {

void show_fixed_toast(AdwToastOverlay* toast_overlay,
                      const std::string& text,
                      const AdwToastPriority& priority = ADW_TOAST_PRIORITY_HIGH);

void show_autohiding_toast(AdwToastOverlay* toast_overlay,
                           const std::string& text,
                           const uint& timeout = 5U,
                           const AdwToastPriority& priority = ADW_TOAST_PRIORITY_HIGH);

auto missing_plugin_box(const std::string& base_name, const std::string& package) -> GtkWidget*;

void show_simple_message_dialog(GtkWidget* parent, const std::string& title, const std::string& descr);

auto parse_spinbutton_output(GtkSpinButton* button, const char* unit, const bool& lower_bound = true) -> gboolean;

auto parse_spinbutton_input(GtkSpinButton* button, gdouble* new_value, const bool& lower_bound = true) -> gint;

auto get_new_filter_serial() -> uint;

void set_ignore_filter_idle_add(const uint& serial, const bool& state);

auto get_ignore_filter_idle_add(const uint& serial) -> bool;

void save_user_locale();

auto get_user_locale() -> std::locale;

auto get_plugin_credit_translated(const std::string& plugin_package) -> std::string;

void update_level(GtkLevelBar* w_left,
                  GtkLabel* w_left_label,
                  GtkLevelBar* w_right,
                  GtkLabel* w_right_label,
                  const float& left,
                  const float& right);

void append_to_string_list(GtkStringList* string_list, const std::string& name);

void remove_from_string_list(GtkStringList* string_list, const std::string& name);

void init_global_app_settings();

void unref_global_app_settings();

auto get_global_app_settings() -> GSettings*;

template <StringLiteralWrapper sl_wrapper, bool lower_bound = true>
void prepare_spinbutton(GtkSpinButton* button) {
  if (button == nullptr) {
    util::warning("Null pointer provided: Spinbutton widget not prepared.");

    return;
  }

  g_signal_connect(button, "output", G_CALLBACK(+[](GtkSpinButton* button, gpointer user_data) {
                     return parse_spinbutton_output(button, sl_wrapper.msg.data(), lower_bound);
                   }),
                   nullptr);

  g_signal_connect(button, "input", G_CALLBACK(+[](GtkSpinButton* button, gdouble* new_value, gpointer user_data) {
                     return parse_spinbutton_input(button, new_value, lower_bound);
                   }),
                   nullptr);
}

template <StringLiteralWrapper sl_wrapper>
void prepare_scale(GtkScale* scale) {
  /*
    The sanitizer caught a "use after free" inside this function. As the problem happens randomly and is hard to
    reproduce I am not sure about what could be the cause yet. So for now I am just checking for null pointers.
  */

  if (scale == nullptr) {
    util::warning("Null pointer provided: Scale widget not prepared.");

    return;
  }

  gtk_scale_set_format_value_func(
      scale,
      (GtkScaleFormatValueFunc) +
          [](GtkScale* scale, double value, gpointer user_data) {
            if (scale == nullptr) {
              return g_strdup("");
            }

            auto precision = gtk_scale_get_digits(scale);
            auto unit = sl_wrapper.msg.data();

            using namespace std::string_literals;

            auto text = fmt::format(ui::get_user_locale(), "{0:.{1}Lf}{2}", value, precision,
                                    ((unit != nullptr) ? " "s + unit : ""));

            return g_strdup(text.c_str());
          },
      nullptr, nullptr);
}

template <StringLiteralWrapper key_wrapper, bool lower_bound = true, typename... Targs>
void prepare_spinbuttons(Targs... button) {
  (prepare_spinbutton<key_wrapper, lower_bound>(button), ...);
}

template <StringLiteralWrapper key_wrapper, typename... Targs>
void prepare_scales(Targs... scale) {
  (prepare_scale<key_wrapper>(scale), ...);
}

template <typename T>
void gsettings_bind_widget(GSettings* settings,
                           const char* key,
                           T widget,
                           GSettingsBindFlags flags = G_SETTINGS_BIND_DEFAULT) {
  static_assert(std::is_same_v<T, GtkSpinButton*> || std::is_same_v<T, GtkToggleButton*> ||
                std::is_same_v<T, GtkSwitch*> || std::is_same_v<T, GtkComboBoxText*> || std::is_same_v<T, GtkScale*>);

  if constexpr (std::is_same_v<T, GtkSpinButton*>) {
    g_settings_bind(settings, key, gtk_spin_button_get_adjustment(widget), "value", flags);
  }

  if constexpr (std::is_same_v<T, GtkScale*>) {
    g_settings_bind(settings, key, gtk_range_get_adjustment(GTK_RANGE(widget)), "value", flags);
  }

  if constexpr (std::is_same_v<T, GtkToggleButton*> || std::is_same_v<T, GtkSwitch*>) {
    g_settings_bind(settings, key, widget, "active", flags);
  }

  if constexpr (std::is_same_v<T, GtkComboBoxText*>) {
    g_settings_bind(settings, key, widget, "active-id", flags);
  }
}

template <StringLiteralWrapper... key_wrapper, typename... Targs>
void gsettings_bind_widgets(GSettings* settings, Targs... widget) {
  (gsettings_bind_widget(settings, key_wrapper.msg.data(), widget), ...);
}

template <typename T>
void gsettings_bind_enum_to_combo_widget(GSettings* settings,
                                         const gchar* key,
                                         T widget,
                                         GSettingsBindFlags flags = G_SETTINGS_BIND_DEFAULT) {
  static_assert(std::is_same_v<T, GtkDropDown*> || std::is_same_v<T, AdwComboRow*>);

  struct Data {
    GSettings* settings;
    const gchar* key;
  };

  g_settings_bind_with_mapping(
      settings, key, widget, "selected", flags,
      +[](GValue* value, GVariant* variant, gpointer user_data) {
        auto* d = static_cast<Data*>(user_data);

        g_value_set_uint(value, static_cast<guint>(g_settings_get_enum(d->settings, d->key)));

        return 1;
      },
      +[](const GValue* value, const GVariantType* expected_type, gpointer user_data) {
        auto* d = static_cast<Data*>(user_data);

        g_settings_set_enum(d->settings, d->key, static_cast<gint>(g_value_get_uint(value)));

        const auto str_value = util::gsettings_get_string(d->settings, d->key);

        return g_variant_new_string(str_value.c_str());
      },
      new Data({.settings = settings, .key = key}),
      +[](gpointer user_data) {
        auto* d = static_cast<Data*>(user_data);

        delete d;
      });
}

}  // namespace ui
