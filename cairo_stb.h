#ifndef CAIRO_STB_H_
#define CAIRO_STB_H_

#include <cairo/cairo.h>

#include <utility>

class CairoStb final {
public:
    using size_type = int;

    struct Dimensions {
        int width{};
        int height{};
    };

    CairoStb() = default;
    CairoStb(const CairoStb &other_img);
    CairoStb(CairoStb &&other_img) noexcept;
    CairoStb(const unsigned char *img_data, const size_type img_size);
    CairoStb &operator=(CairoStb &&other_img) noexcept;
    CairoStb &operator=(const CairoStb &other_img);
    operator cairo_surface_t *() const noexcept;
    void load_image(const unsigned char *img_data, const size_type img_size);
    Dimensions dimensions() const noexcept;
    size_type size() const noexcept;
    cairo_surface_t *get_surf() const noexcept;
    ~CairoStb();

private:
    void free_image_memory() noexcept;
    void create_cairo_compatible_surface(const unsigned char *raw_pixel_data);

    cairo_surface_t *cairo_surface{nullptr};
    Dimensions image_dimensions;
    size_type image_size{};

    static constexpr size_type image_byte_pixel_amnt = 4;  // RGBA
    static constexpr auto cairo_surface_type = CAIRO_FORMAT_ARGB32;
};

#endif  // CAIRO_STB_H_