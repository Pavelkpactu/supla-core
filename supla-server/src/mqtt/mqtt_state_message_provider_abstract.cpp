/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <mqtt_state_message_provider_abstract.h>
#include <stdlib.h>
#include <string.h>

supla_mqtt_state_message_provider_abstract::
    supla_mqtt_state_message_provider_abstract(void)
    : supla_mqtt_message_provider() {
  this->user_id = 0;
  this->device_id = 0;
  this->channel_id = 0;
  this->cvalue = NULL;
  this->user_email[0] = 0;
  this->em = NULL;
  this->icm = NULL;
}

supla_mqtt_state_message_provider_abstract::
    ~supla_mqtt_state_message_provider_abstract(void) {
  if (cvalue) {
    free(cvalue);
  }

  if (em) {
    delete em;
  }

  if (icm) {
    delete icm;
  }
}

int supla_mqtt_state_message_provider_abstract::get_user_id(void) {
  return user_id;
}

int supla_mqtt_state_message_provider_abstract::get_device_id(void) {
  return device_id;
}

int supla_mqtt_state_message_provider_abstract::get_channel_id(void) {
  return channel_id;
}

void supla_mqtt_state_message_provider_abstract::set_data(int user_id,
                                                          int device_id,
                                                          int channel_id) {
  const char *email = _get_user_email();
  snprintf(this->user_email, SUPLA_EMAIL_MAXSIZE, "%s", email ? email : "");
  this->user_id = user_id;
  this->device_id = device_id;
  this->channel_id = channel_id;

  if (cvalue) {
    free(cvalue);
    cvalue = NULL;
  }

  if (em) {
    delete em;
    em = NULL;
  }

  if (icm) {
    delete icm;
    icm = NULL;
  }
}

channel_complex_value *
supla_mqtt_state_message_provider_abstract::get_complex_value(void) {
  if (cvalue == NULL) {
    cvalue = _get_complex_value(user_id, device_id, channel_id);
  }

  return cvalue;
}

supla_channel_electricity_measurement *
supla_mqtt_state_message_provider_abstract::get_electricity_measurement(void) {
  if (em == NULL) {
    em = _get_electricity_measurement();
  }

  return em;
}

supla_channel_ic_measurement *
supla_mqtt_state_message_provider_abstract::get_impulse_counter_measurement(
    void) {
  if (icm == NULL) {
    icm = _get_impulse_counter_measurement();
  }

  return icm;
}

void supla_mqtt_state_message_provider_abstract::get_temperature(
    const channel_complex_value *cvalue, char *buf, size_t buf_size) {
  snprintf(buf, buf_size, "%f", cvalue->temperature);
}

void supla_mqtt_state_message_provider_abstract::get_humidity(
    const channel_complex_value *cvalue, char *buf, size_t buf_size) {
  snprintf(buf, buf_size, "%f", cvalue->humidity);
}

void supla_mqtt_state_message_provider_abstract::get_brightness(
    char brightness, char *buf, size_t buf_size) {
  snprintf(buf, buf_size, "%i", brightness);
}

void supla_mqtt_state_message_provider_abstract::get_color(
    const channel_complex_value *cvalue, char *buf, size_t buf_size) {
  snprintf(buf, buf_size, "0x%02X%02X%02X",
           ((unsigned char *)&cvalue->color)[2],
           ((unsigned char *)&cvalue->color)[1],
           ((unsigned char *)&cvalue->color)[0]);
}

void supla_mqtt_state_message_provider_abstract::get_valve_closed(
    const channel_complex_value *cvalue, char *buf, size_t buf_size) {
  if (cvalue->function == SUPLA_CHANNELFNC_VALVE_PERCENTAGE) {
    snprintf(buf, buf_size, "%i", cvalue->valve_value.closed_percent);
  } else {
    snprintf(buf, buf_size, "%s",
             cvalue->valve_value.closed ? "true" : "false");
  }
}

bool supla_mqtt_state_message_provider_abstract::get_rs_message_at_index(
    const channel_complex_value *cvalue, unsigned short index,
    const char *topic_prefix, char **topic_name, void **message,
    size_t *message_size) {
  char value[50];
  value[0] = 0;

  switch (index) {
    case 1:
      return create_message(topic_prefix, user_email, topic_name, message,
                            message_size, cvalue->hi ? "true" : "false", false,
                            "channels/%i/state/hi", get_channel_id());
    case 2:
      return create_message(topic_prefix, user_email, topic_name, message,
                            message_size, cvalue->shut == -1 ? "true" : "false",
                            false, "channels/%i/state/is_calibrating",
                            get_channel_id());
    case 3: {
      short shut = cvalue->shut;
      if (shut < 0) {
        shut = 0;
      } else if (shut > 100) {
        shut = 100;
      }
      snprintf(value, sizeof(value), "%i", shut);
      return create_message(topic_prefix, user_email, topic_name, message,
                            message_size, value, false,
                            "channels/%i/state/shut", get_channel_id());
    }
  }

  return false;
}

bool supla_mqtt_state_message_provider_abstract::get_lck_message_at_index(
    const channel_complex_value *cvalue, unsigned short index,
    const char *topic_prefix, char **topic_name, void **message,
    size_t *message_size) {
  if (index == 1) {
    return create_message(topic_prefix, user_email, topic_name, message,
                          message_size, cvalue->hi ? "true" : "false", false,
                          "channels/%i/state/hi", get_channel_id());
  }

  return false;
}

bool supla_mqtt_state_message_provider_abstract::get_gate_message_at_index(
    const channel_complex_value *cvalue, unsigned short index,
    const char *topic_prefix, char **topic_name, void **message,
    size_t *message_size) {
  switch (index) {
    case 1:
      return create_message(topic_prefix, user_email, topic_name, message,
                            message_size, cvalue->hi ? "true" : "false", false,
                            "channels/%i/state/hi", get_channel_id());
    case 2:
      return create_message(topic_prefix, user_email, topic_name, message,
                            message_size,
                            cvalue->partially_closed ? "true" : "false", false,
                            "channels/%i/state/partial_hi", get_channel_id());
  }

  return false;
}

bool supla_mqtt_state_message_provider_abstract::get_onoff_message_at_index(
    bool on, unsigned short index, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  if (index == 1) {
    return create_message(topic_prefix, user_email, topic_name, message,
                          message_size, on ? "true" : "false", false,
                          "channels/%i/state/on", get_channel_id());
  }

  return false;
}

bool supla_mqtt_state_message_provider_abstract::get_depth_message_at_index(
    const channel_complex_value *cvalue, unsigned short index,
    const char *topic_prefix, char **topic_name, void **message,
    size_t *message_size) {
  if (index == 1) {
    char value[50];
    value[0] = 0;

    snprintf(value, sizeof(value), "%f", cvalue->depth);
    return create_message(topic_prefix, user_email, topic_name, message,
                          message_size, value, false, "channels/%i/state/depth",
                          get_channel_id());
  }

  return false;
}

bool supla_mqtt_state_message_provider_abstract::get_distance_message_at_index(
    const channel_complex_value *cvalue, unsigned short index,
    const char *topic_prefix, char **topic_name, void **message,
    size_t *message_size) {
  if (index == 1) {
    char value[50];
    value[0] = 0;

    snprintf(value, sizeof(value), "%f", cvalue->distance);
    return create_message(topic_prefix, user_email, topic_name, message,
                          message_size, value, false,
                          "channels/%i/state/distance", get_channel_id());
  }

  return false;
}

bool supla_mqtt_state_message_provider_abstract::
    get_doublevalue_message_at_index(const channel_complex_value *cvalue,
                                     unsigned short index,
                                     const char *topic_prefix,
                                     char **topic_name, void **message,
                                     size_t *message_size) {
  if (index == 1) {
    char value[50];
    value[0] = 0;

    switch (cvalue->function) {
      case SUPLA_CHANNELFNC_PRESSURESENSOR:
        snprintf(value, sizeof(value), "%f", cvalue->pressure);
        break;
      case SUPLA_CHANNELFNC_RAINSENSOR:
        snprintf(value, sizeof(value), "%f", cvalue->rain);
        break;
      case SUPLA_CHANNELFNC_WEIGHTSENSOR:
        snprintf(value, sizeof(value), "%f", cvalue->weight);
        break;
      case SUPLA_CHANNELFNC_WINDSENSOR:
        snprintf(value, sizeof(value), "%f", cvalue->wind);
        break;
    }

    return create_message(topic_prefix, user_email, topic_name, message,
                          message_size, value, false, "channels/%i/state/value",
                          get_channel_id());
  }

  return false;
}

bool supla_mqtt_state_message_provider_abstract::get_sensor_message_at_index(
    const channel_complex_value *cvalue, unsigned short index,
    const char *topic_prefix, char **topic_name, void **message,
    size_t *message_size) {
  if (index == 1) {
    return create_message(topic_prefix, user_email, topic_name, message,
                          message_size, cvalue->hi ? "true" : "false", false,
                          "channels/%i/state/hi", get_channel_id());
  }

  return false;
}

bool supla_mqtt_state_message_provider_abstract::
    get_temperature_message_at_index(const channel_complex_value *cvalue,
                                     unsigned short index,
                                     unsigned short expected_index,
                                     const char *topic_prefix,
                                     char **topic_name, void **message,
                                     size_t *message_size) {
  if (index == expected_index) {
    char value[50];
    value[0] = 0;

    get_temperature(cvalue, value, sizeof(value));
    return create_message(topic_prefix, user_email, topic_name, message,
                          message_size, value, false,
                          "channels/%i/state/temperature", get_channel_id());
  }

  return false;
}

bool supla_mqtt_state_message_provider_abstract::get_humidity_message_at_index(
    const channel_complex_value *cvalue, unsigned short index,
    unsigned short expected_index, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  if (index == expected_index) {
    char value[50];
    value[0] = 0;

    get_humidity(cvalue, value, sizeof(value));
    return create_message(topic_prefix, user_email, topic_name, message,
                          message_size, value, false,
                          "channels/%i/state/humidity", get_channel_id());
  }

  return false;
}

bool supla_mqtt_state_message_provider_abstract::get_brightness_message(
    const channel_complex_value *cvalue, const char *topic_prefix,
    char **topic_name, void **message, size_t *message_size) {
  char value[50];
  value[0] = 0;

  get_brightness(cvalue->brightness, value, sizeof(value));
  return create_message(topic_prefix, user_email, topic_name, message,
                        message_size, value, false,
                        "channels/%i/state/brightness", get_channel_id());
}

bool supla_mqtt_state_message_provider_abstract::get_color_brightness_message(
    const channel_complex_value *cvalue, const char *topic_prefix,
    char **topic_name, void **message, size_t *message_size) {
  char value[50];
  value[0] = 0;

  get_brightness(cvalue->color_brightness, value, sizeof(value));
  return create_message(topic_prefix, user_email, topic_name, message,
                        message_size, value, false,
                        "channels/%i/state/color_brightness", get_channel_id());
}

bool supla_mqtt_state_message_provider_abstract::get_color_message(
    const channel_complex_value *cvalue, const char *topic_prefix,
    char **topic_name, void **message, size_t *message_size) {
  char value[50];
  value[0] = 0;

  get_color(cvalue, value, sizeof(value));
  return create_message(topic_prefix, user_email, topic_name, message,
                        message_size, value, false, "channels/%i/state/color",
                        get_channel_id());
}

bool supla_mqtt_state_message_provider_abstract::get_valve_message_at_index(
    const channel_complex_value *cvalue, unsigned short index,
    const char *topic_prefix, char **topic_name, void **message,
    size_t *message_size) {
  char value[50];
  value[0] = 0;

  switch (index) {
    case 1:
      get_valve_closed(cvalue, value, sizeof(value));
      return create_message(topic_prefix, user_email, topic_name, message,
                            message_size, value, false,
                            "channels/%i/state/closed", get_channel_id());
    case 2:
      return create_message(
          topic_prefix, user_email, topic_name, message, message_size,
          cvalue->valve_value.flags & SUPLA_VALVE_FLAG_MANUALLY_CLOSED
              ? "true"
              : "false",
          false, "channels/%i/state/manually_closed", get_channel_id());
    case 3:
      return create_message(
          topic_prefix, user_email, topic_name, message, message_size,
          cvalue->valve_value.flags & SUPLA_VALVE_FLAG_FLOODING ? "true"
                                                                : "false",
          false, "channels/%i/state/flooding", get_channel_id());
  }

  return false;
}

bool supla_mqtt_state_message_provider_abstract::
    get_impulsecounter_message_at_index(const channel_complex_value *cvalue,
                                        unsigned short index,
                                        const char *topic_prefix,
                                        char **topic_name, void **message,
                                        size_t *message_size) {
  supla_channel_ic_measurement *icm = get_impulse_counter_measurement();
  if (icm == NULL) {
    if (message) {
      message = NULL;
    }

    if (message_size) {
      message_size = NULL;
    }
  }

  char value[50];
  value[0] = 0;

  switch (index) {
    case 1:
      snprintf(value, sizeof(value), "%.2f",
               icm ? (icm->getTotalCost() * 0.01) : 0);
      return create_message(topic_prefix, user_email, topic_name, message,
                            message_size, value, false,
                            "channels/%i/state/total_cost", get_channel_id());
    case 2:
      snprintf(value, sizeof(value), "%.4f",
               icm ? (icm->getPricePerUnit() * 0.0001) : 0);
      return create_message(
          topic_prefix, user_email, topic_name, message, message_size, value,
          false, "channels/%i/state/price_per_unit", get_channel_id());
    case 3:
      snprintf(value, sizeof(value), "%i", icm ? icm->getImpulsesPerUnit() : 0);
      return create_message(
          topic_prefix, user_email, topic_name, message, message_size, value,
          false, "channels/%i/state/impulses_per_unit", get_channel_id());
    case 4:
      snprintf(value, sizeof(value), "%llu", icm ? icm->getCounter() : 0);
      return create_message(topic_prefix, user_email, topic_name, message,
                            message_size, value, false,
                            "channels/%i/state/counter", get_channel_id());
    case 5:
      snprintf(value, sizeof(value), "%.3f",
               icm ? (icm->getCalculatedValue() * 0.001) : 0);
      return create_message(
          topic_prefix, user_email, topic_name, message, message_size, value,
          false, "channels/%i/state/calculated_value", get_channel_id());
    case 6:
      return create_message(topic_prefix, user_email, topic_name, message,
                            message_size, icm ? icm->getCurrency() : "", false,
                            "channels/%i/state/currency", get_channel_id());
    case 7:
      return create_message(topic_prefix, user_email, topic_name, message,
                            message_size, icm ? icm->getCustomUnit() : "",
                            false, "channels/%i/state/unit", get_channel_id());
  }

  return false;
}

void supla_mqtt_state_message_provider_abstract::verify_flag(
    supla_channel_electricity_measurement **em, int flags, int flag1, int flag2,
    void **message, size_t *message_size) {
  if ((flags & flag1) == 0 && (flags & flag2) == 0) {
    em = NULL;
    if (message) {
      *message = NULL;
    }
    if (message_size) {
      *message_size = 0;
    }
  }
}

bool supla_mqtt_state_message_provider_abstract::
    get_electricitymeter_message_at_index(const channel_complex_value *cvalue,
                                          unsigned short index,
                                          const char *topic_prefix,
                                          char **topic_name, void **message,
                                          size_t *message_size) {
  supla_channel_electricity_measurement *em = get_electricity_measurement();

  TElectricityMeter_ExtendedValue_V2 em_ev;
  memset(&em_ev, 0, sizeof(TElectricityMeter_ExtendedValue_V2));

  if (em) {
    em->getMeasurement(&em_ev);
    if (em_ev.m_count == 0) {
      em = NULL;
    }
  }

  if (em == NULL) {
    if (message) {
      message = NULL;
    }

    if (message_size) {
      message_size = NULL;
    }
  }

  char value[50];
  value[0] = 0;

  short phase = (index - 8) / 12;

  switch (index) {
    case 1:
      snprintf(value, sizeof(value), "%.2f",
               em ? (em_ev.total_cost * 0.01) : 0);
      return create_message(topic_prefix, user_email, topic_name, message,
                            message_size, value, false,
                            "channels/%i/state/total_cost", get_channel_id());
    case 2:
      snprintf(value, sizeof(value), "%.2f",
               em ? (em_ev.total_cost_balanced * 0.01) : 0);
      return create_message(
          topic_prefix, user_email, topic_name, message, message_size, value,
          false, "channels/%i/state/total_cost_balanced", get_channel_id());

    case 3:
      snprintf(value, sizeof(value), "%.4f",
               em ? (em_ev.price_per_unit * 0.0001) : 0);
      return create_message(
          topic_prefix, user_email, topic_name, message, message_size, value,
          false, "channels/%i/state/price_per_unit", get_channel_id());

    case 4:
      if (em) {
        em->getCurrency(value);
      }
      return create_message(topic_prefix, user_email, topic_name, message,
                            message_size, value, false,
                            "channels/%i/state/currency", get_channel_id());

    case 5:
      snprintf(value, sizeof(value), "%u", em ? em_ev.measured_values : 0);
      // This topic should be called "measured_values" but for compatibility
      // with the rest API it has been changed to "support"
      return create_message(topic_prefix, user_email, topic_name, message,
                            message_size, value, false,
                            "channels/%i/state/support", get_channel_id());

    case 6:
      verify_flag(&em, em_ev.measured_values,
                  EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED, 0, message,
                  message_size);
      snprintf(value, sizeof(value), "%.5f",
               em ? (em_ev.total_forward_active_energy_balanced * 0.00001) : 0);
      return create_message(
          topic_prefix, user_email, topic_name, message, message_size, value,
          false, "channels/%i/state/total_forward_active_energy_balanced",
          get_channel_id());

    case 7:
      verify_flag(&em, em_ev.measured_values,
                  EM_VAR_REVERSE_ACTIVE_ENERGY_BALANCED, 0, message,
                  message_size);
      snprintf(value, sizeof(value), "%.5f",
               em ? (em_ev.total_reverse_active_energy_balanced * 0.00001) : 0);
      return create_message(
          topic_prefix, user_email, topic_name, message, message_size, value,
          false, "channels/%i/state/total_reverse_active_energy_balanced",
          get_channel_id());

    case 8:
    case 20:
    case 32:
      verify_flag(&em, em_ev.measured_values, EM_VAR_FORWARD_ACTIVE_ENERGY, 0,
                  message, message_size);
      snprintf(value, sizeof(value), "%.5f",
               em ? (em_ev.total_forward_active_energy[phase] * 0.00001) : 0);
      return create_message(
          topic_prefix, user_email, topic_name, message, message_size, value,
          false, "channels/%i/state/phases/%i/total_forward_active_energy",
          get_channel_id(), phase + 1);

    case 9:
    case 21:
    case 33:
      verify_flag(&em, em_ev.measured_values, EM_VAR_REVERSE_ACTIVE_ENERGY, 0,
                  message, message_size);
      snprintf(value, sizeof(value), "%.5f",
               em ? (em_ev.total_reverse_active_energy[phase] * 0.00001) : 0);
      return create_message(
          topic_prefix, user_email, topic_name, message, message_size, value,
          false, "channels/%i/state/phases/%i/total_reverse_active_energy",
          get_channel_id(), phase + 1);

    case 10:
    case 22:
    case 34:
      verify_flag(&em, em_ev.measured_values, EM_VAR_FORWARD_REACTIVE_ENERGY, 0,
                  message, message_size);
      snprintf(value, sizeof(value), "%.5f",
               em ? (em_ev.total_forward_reactive_energy[phase] * 0.00001) : 0);
      return create_message(
          topic_prefix, user_email, topic_name, message, message_size, value,
          false, "channels/%i/state/phases/%i/total_forward_reactive_energy",
          get_channel_id(), phase + 1);

    case 11:
    case 23:
    case 35:
      verify_flag(&em, em_ev.measured_values, EM_VAR_REVERSE_REACTIVE_ENERGY, 0,
                  message, message_size);
      snprintf(value, sizeof(value), "%.5f",
               em ? (em_ev.total_reverse_reactive_energy[phase] * 0.00001) : 0);
      return create_message(
          topic_prefix, user_email, topic_name, message, message_size, value,
          false, "channels/%i/state/phases/%i/total_reverse_reactive_energy",
          get_channel_id(), phase + 1);

    case 12:
    case 24:
    case 36:
      verify_flag(&em, em_ev.measured_values, EM_VAR_FREQ, 0, message,
                  message_size);
      snprintf(value, sizeof(value), "%.2f", em ? (em_ev.m[0].freq * 0.01) : 0);
      return create_message(topic_prefix, user_email, topic_name, message,
                            message_size, value, false,
                            "channels/%i/state/phases/%i/frequency",
                            get_channel_id(), phase + 1);

    case 13:
    case 25:
    case 37:
      verify_flag(&em, em_ev.measured_values, EM_VAR_VOLTAGE, 0, message,
                  message_size);
      snprintf(value, sizeof(value), "%.2f",
               em ? (em_ev.m[0].voltage[phase] * 0.01) : 0);
      return create_message(topic_prefix, user_email, topic_name, message,
                            message_size, value, false,
                            "channels/%i/state/phases/%i/voltage",
                            get_channel_id(), phase + 1);

    case 14:
    case 26:
    case 38: {
      verify_flag(&em, em_ev.measured_values, EM_VAR_CURRENT,
                  EM_VAR_CURRENT_OVER_65A, message, message_size);
      unsigned int current = em_ev.m[0].current[phase];

      if ((em_ev.measured_values & EM_VAR_CURRENT_OVER_65A) &&
          !(em_ev.measured_values & EM_VAR_CURRENT)) {
        current *= 10;
      }

      snprintf(value, sizeof(value), "%.3f", em ? (current * 0.001) : 0);
      return create_message(topic_prefix, user_email, topic_name, message,
                            message_size, value, false,
                            "channels/%i/state/phases/%i/current",
                            get_channel_id(), phase + 1);
    }
    case 15:
    case 27:
    case 39:
      verify_flag(&em, em_ev.measured_values, EM_VAR_POWER_ACTIVE, 0, message,
                  message_size);
      snprintf(value, sizeof(value), "%.5f",
               em ? (em_ev.m[0].power_active[phase] * 0.00001) : 0);
      return create_message(topic_prefix, user_email, topic_name, message,
                            message_size, value, false,
                            "channels/%i/state/phases/%i/power_active",
                            get_channel_id(), phase + 1);
    case 16:
    case 28:
    case 40:
      verify_flag(&em, em_ev.measured_values, EM_VAR_POWER_REACTIVE, 0, message,
                  message_size);
      snprintf(value, sizeof(value), "%.5f",
               em ? (em_ev.m[0].power_reactive[phase] * 0.00001) : 0);
      return create_message(topic_prefix, user_email, topic_name, message,
                            message_size, value, false,
                            "channels/%i/state/phases/%i/power_reactive",
                            get_channel_id(), phase + 1);
    case 17:
    case 29:
    case 41:
      verify_flag(&em, em_ev.measured_values, EM_VAR_POWER_APPARENT, 0, message,
                  message_size);
      snprintf(value, sizeof(value), "%.5f",
               em ? (em_ev.m[0].power_apparent[phase] * 0.00001) : 0);
      return create_message(topic_prefix, user_email, topic_name, message,
                            message_size, value, false,
                            "channels/%i/state/phases/%i/power_apparent",
                            get_channel_id(), phase + 1);
    case 18:
    case 30:
    case 42:
      verify_flag(&em, em_ev.measured_values, EM_VAR_POWER_FACTOR, 0, message,
                  message_size);
      snprintf(value, sizeof(value), "%.3f",
               em ? (em_ev.m[0].power_factor[phase] * 0.001) : 0);
      return create_message(topic_prefix, user_email, topic_name, message,
                            message_size, value, false,
                            "channels/%i/state/phases/%i/power_factor",
                            get_channel_id(), phase + 1);
    case 19:
    case 31:
    case 43:
      verify_flag(&em, em_ev.measured_values, EM_VAR_PHASE_ANGLE, 0, message,
                  message_size);
      snprintf(value, sizeof(value), "%.1f",
               em ? (em_ev.m[0].phase_angle[phase] * 0.1) : 0);
      return create_message(topic_prefix, user_email, topic_name, message,
                            message_size, value, false,
                            "channels/%i/state/phases/%i/phase_angle",
                            get_channel_id(), phase + 1);
  }

  return false;
}

bool supla_mqtt_state_message_provider_abstract::get_message_at_index(
    unsigned short index, const char *topic_prefix, char **topic_name,
    void **message, size_t *message_size) {
  channel_complex_value *cvalue = get_complex_value();
  if (cvalue == NULL) {
    return false;
  }

  if (index == 0) {
    return create_message(topic_prefix, user_email, topic_name, message,
                          message_size, cvalue->online ? "true" : "false",
                          false, "channels/%i/state/connected",
                          get_channel_id());
  }

  if (!cvalue->online) {
    message = NULL;
    if (message_size) {
      *message_size = 0;
    }
  }

  switch (cvalue->function) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:
      return get_rs_message_at_index(cvalue, index, topic_prefix, topic_name,
                                     message, message_size);

    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
      return get_gate_message_at_index(cvalue, index, topic_prefix, topic_name,
                                       message, message_size);

    case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
      return get_lck_message_at_index(cvalue, index, topic_prefix, topic_name,
                                      message, message_size);

    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
    case SUPLA_CHANNELFNC_STAIRCASETIMER:
    case SUPLA_CHANNELFNC_THERMOSTAT:
    case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS:
      return get_onoff_message_at_index(cvalue->hi > 0, index, topic_prefix,
                                        topic_name, message, message_size);

    case SUPLA_CHANNELFNC_DEPTHSENSOR:
      return get_depth_message_at_index(cvalue, index, topic_prefix, topic_name,
                                        message, message_size);

    case SUPLA_CHANNELFNC_DISTANCESENSOR:
      return get_distance_message_at_index(cvalue, index, topic_prefix,
                                           topic_name, message, message_size);

    case SUPLA_CHANNELFNC_PRESSURESENSOR:
    case SUPLA_CHANNELFNC_RAINSENSOR:
    case SUPLA_CHANNELFNC_WEIGHTSENSOR:
    case SUPLA_CHANNELFNC_WINDSENSOR:
      return get_doublevalue_message_at_index(
          cvalue, index, topic_prefix, topic_name, message, message_size);

    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROOFWINDOW:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:
    case SUPLA_CHANNELFNC_MAILSENSOR:
    case SUPLA_CHANNELFNC_NOLIQUIDSENSOR:
      return get_sensor_message_at_index(cvalue, index, topic_prefix,
                                         topic_name, message, message_size);

    case SUPLA_CHANNELFNC_THERMOMETER:
      return get_temperature_message_at_index(
          cvalue, index, 1, topic_prefix, topic_name, message, message_size);

    case SUPLA_CHANNELFNC_HUMIDITY:
      return get_humidity_message_at_index(cvalue, index, 1, topic_prefix,
                                           topic_name, message, message_size);

    case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:
      switch (index) {
        case 1:
          return get_humidity_message_at_index(cvalue, index, 1, topic_prefix,
                                               topic_name, message,
                                               message_size);
        case 2:
          return get_temperature_message_at_index(cvalue, index, 2,
                                                  topic_prefix, topic_name,
                                                  message, message_size);
      }
      break;
    case SUPLA_CHANNELFNC_DIMMER:
      switch (index) {
        case 1:
          return get_onoff_message_at_index(cvalue->brightness > 0, index,
                                            topic_prefix, topic_name, message,
                                            message_size);

        case 2:
          return get_brightness_message(cvalue, topic_prefix, topic_name,
                                        message, message_size);
      }
      break;
    case SUPLA_CHANNELFNC_RGBLIGHTING:
      switch (index) {
        case 1:
          return get_onoff_message_at_index(cvalue->color_brightness > 0, index,
                                            topic_prefix, topic_name, message,
                                            message_size);

        case 2:
          return get_color_message(cvalue, topic_prefix, topic_name, message,
                                   message_size);
        case 3:
          return get_color_brightness_message(cvalue, topic_prefix, topic_name,
                                              message, message_size);
      }
      break;
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
      switch (index) {
        case 1:
          return get_onoff_message_at_index(
              cvalue->brightness > 0 || cvalue->color_brightness > 0, index,
              topic_prefix, topic_name, message, message_size);

        case 2:
          return get_color_message(cvalue, topic_prefix, topic_name, message,
                                   message_size);
        case 3:
          return get_color_brightness_message(cvalue, topic_prefix, topic_name,
                                              message, message_size);
        case 4:
          return get_brightness_message(cvalue, topic_prefix, topic_name,
                                        message, message_size);
      }
      break;
    case SUPLA_CHANNELFNC_VALVE_OPENCLOSE:
    case SUPLA_CHANNELFNC_VALVE_PERCENTAGE:
      return get_valve_message_at_index(cvalue, index, topic_prefix, topic_name,
                                        message, message_size);

    case SUPLA_CHANNELFNC_ELECTRICITY_METER:
#ifdef SERVER_VERSION_23
      if (cvalue->channel_type == SUPLA_CHANNELTYPE_ELECTRICITY_METER) {
        return get_electricitymeter_message_at_index(
            cvalue, index, topic_prefix, topic_name, message, message_size);
      } else if (cvalue->channel_type == SUPLA_CHANNELTYPE_IMPULSE_COUNTER) {
        return get_impulsecounter_message_at_index(
            cvalue, index, topic_prefix, topic_name, message, message_size);
      }
#else
      return get_electricitymeter_message_at_index(
          cvalue, index, topic_prefix, topic_name, message, message_size);
#endif /*SERVER_VERSION_23*/
      break;
    case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
    case SUPLA_CHANNELFNC_IC_GAS_METER:
    case SUPLA_CHANNELFNC_IC_WATER_METER:
    case SUPLA_CHANNELFNC_IC_HEAT_METER:
      return get_impulsecounter_message_at_index(
          cvalue, index, topic_prefix, topic_name, message, message_size);
      break;
  }

  return false;
}