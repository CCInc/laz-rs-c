/*
    Taken with permission from RockRobotic/copc-lib
    https://github.com/RockRobotic/copc-lib/blob/b008d245e2a0facb6d7e9ba738d96f6fcb2cbf29/cpp/src/las/point.cpp
*/

#include <vector>

uint8_t PointBaseByteSize(const int8_t &point_format_id)
{
    switch (point_format_id)
    {
    case 0:
        return 20;
    case 1:
        return 28;
    case 2:
        return 26;
    case 3:
        return 34;
    case 6:
        return 30;
    case 7:
        return 36;
    case 8:
        return 38;
    default:
        fprintf(stderr, "Point format Not supported\n");
        exit(1);
    }
}

bool FormatHasGPSTime(const uint8_t &point_format_id)
{
    switch (point_format_id)
    {
    case 0:
        return false;
    case 1:
        return true;
    case 2:
        return false;
    case 3:
        return true;
    case 6:
    case 7:
    case 8:
        return true;

    default:
        fprintf(stderr, "Point format Not supported\n");
        exit(1);
    }
}

bool FormatHasRGB(const uint8_t &point_format_id)
{
    switch (point_format_id)
    {
    case 0:
    case 1:
        return false;
    case 2:
    case 3:
        return true;
    case 6:
        return false;
    case 7:
    case 8:
        return true;
    default:
        fprintf(stderr, "Point format Not supported\n");
        exit(1);
    }
}

bool FormatHasNIR(const uint8_t &point_format_id)
{
    switch (point_format_id)
    {
    case 0:
    case 1:
    case 2:
    case 3:
        return false;
    case 6:
    case 7:
        return false;
    case 8:
        return true;
    default:
        fprintf(stderr, "Point format Not supported\n");
        exit(1);
    }
}

struct Point
{
    Point(const int8_t point_format_id, const uint16_t &num_extra_bytes){
        if (point_format_id > 5)
            extended_point_type_ = true;

        has_gps_time_ = FormatHasGPSTime(point_format_id);
        has_rgb_ = FormatHasRGB(point_format_id);
        has_nir_ = FormatHasNIR(point_format_id);

        extra_bytes_.resize(num_extra_bytes, 0);
    }

    int32_t x_{};
    int32_t y_{};
    int32_t z_{};
    uint16_t intensity_{};
    uint8_t returns_flags_eof_{};
    uint8_t classification_{};
    int8_t scan_angle_rank_{};
    uint8_t user_data_{};
    uint16_t point_source_id_{};

    // LAS 1.4 only
    bool extended_point_type_{false};
    uint8_t extended_returns_{};
    uint8_t extended_flags_{};
    uint8_t extended_classification_{};
    int16_t extended_scan_angle_{};

    double gps_time_{};
    uint16_t rgb_[3]{};
    uint16_t nir_{};

    bool has_gps_time_{false};
    bool has_rgb_{false};
    bool has_nir_{false};

    std::vector<uint8_t> extra_bytes_;
};

template <typename T> T unpack(uint8_t *point_data)
{
    T out = *(T *)(point_data);
    point_data += sizeof(T);
    return out;
}

Point UnpackPoint(uint8_t *point_data, const int8_t &point_format_id, const uint16_t &point_record_len)
{
    uint16_t num_extra_bytes = point_record_len - PointBaseByteSize(point_format_id);
    Point p(point_format_id, num_extra_bytes);

    p.x_ = unpack<int32_t>(point_data);
    p.y_ = unpack<int32_t>(point_data);
    p.z_ = unpack<int32_t>(point_data);
    p.intensity_ = unpack<uint16_t>(point_data);
    if (p.extended_point_type_)
    {
        p.extended_returns_ = unpack<uint8_t>(point_data);
        p.extended_flags_ = unpack<uint8_t>(point_data);
        p.extended_classification_ = unpack<uint8_t>(point_data);
        p.user_data_ = unpack<uint8_t>(point_data);
        p.extended_scan_angle_ = unpack<int16_t>(point_data);
    }
    else
    {
        p.returns_flags_eof_ = unpack<uint8_t>(point_data);
        p.classification_ = unpack<uint8_t>(point_data);
        p.scan_angle_rank_ = unpack<int8_t>(point_data);
        p.user_data_ = unpack<uint8_t>(point_data);
    }
    p.point_source_id_ = unpack<uint16_t>(point_data);

    if (p.has_gps_time_)
    {
        p.gps_time_ = unpack<double>(point_data);
    }
    if (p.has_rgb_)
    {
        p.rgb_[0] = unpack<uint16_t>(point_data);
        p.rgb_[1] = unpack<uint16_t>(point_data);
        p.rgb_[2] = unpack<uint16_t>(point_data);
    }
    if (p.has_nir_)
    {
        p.nir_ = unpack<uint16_t>(point_data);
    }

    for (uint32_t i = 0; i < num_extra_bytes; i++)
    {
        p.extra_bytes_[i] = unpack<uint8_t>(point_data);
    }
    return p;
}