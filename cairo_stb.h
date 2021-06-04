#ifndef CAIRO_STB_H_
#define CAIRO_STB_H_

#include <cairo/cairo.h>

#include <utility>
#include <vector>

class CairoStb final {
public:
    using size_type = int;

    struct Dimensions {
        size_type width{};
        size_type height{};
    };

    CairoStb() = delete;
    CairoStb(const CairoStb &other_img);
    CairoStb(CairoStb &&other_img) noexcept;
    CairoStb(const unsigned char *img_data, const size_type img_size);
    CairoStb(const std::vector<unsigned char> &img_data);
    CairoStb &operator=(CairoStb &&other_img) noexcept;
    CairoStb &operator=(const CairoStb &other_img);
    operator cairo_surface_t *() const noexcept;
    void load_image(const unsigned char *img_data, const size_type img_size);
    Dimensions dimensions() const noexcept;
    cairo_surface_t *get_surf() const noexcept;
    ~CairoStb();

private:
    void free_image_memory() noexcept;
    void create_cairo_compatible_surface(unsigned char *raw_pixel_data);

    cairo_surface_t *cairo_surface{nullptr};
    Dimensions image_dimensions;
    size_type image_size{};

    static constexpr size_type IMAGE_BYTE_PIXEL_AMNT = 4;  // RGBA
    static constexpr auto CAIRO_SURFACE_TYPE = CAIRO_FORMAT_ARGB32;
};

#endif  // CAIRO_STB_H_