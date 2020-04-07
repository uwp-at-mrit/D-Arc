#pragma once

#include "syslog.hpp"

#ifdef _DEBUG
static WarGrey::GYDM::Log default_logging_level = WarGrey::GYDM::Log::Debug;
#else
static WarGrey::GYDM::Log default_logging_level = WarGrey::GYDM::Log::Info;
#endif

static WarGrey::GYDM::Log default_gps_logging_level = WarGrey::GYDM::Log::Info;

static Platform::String^ rsyslog_host = "239.255.203.113";
static unsigned short rsyslog_port = 1618;

/*************************************************************************************************/
static const unsigned int frame_per_second = 5U;

static const unsigned int diagnostics_caption_background = 0x8FBC8F;
static const unsigned int diagnostics_caption_foreground = 0xF8F8FF;
static const unsigned int diagnostics_region_background = 0x414141U;
static const unsigned int diagnostics_alarm_background = 0x141414U;

static const float large_metrics_font_size = 24.0F;
static const float normal_metrics_font_size = 22.0F;
static const float small_metrics_font_size = 20.0F;

static const float large_font_size = 18.0F;
static const float normal_font_size = 16.0F;
static const float small_font_size = 14.0F;
static const float tiny_font_size = 12.0F;
