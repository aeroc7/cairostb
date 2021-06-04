#include "cairo_stb.h"

#include <stb_image.h>

#include <cstring>
#include <stdexcept>
#include <string>

CairoStb::CairoStb(const CairoStb &other_img) { *this = operator=(other_img); }

CairoStb::CairoStb(CairoStb &&other_img) noexcept
    : cairo_surface(std::exchange(other_img.cairo_surface, nullptr)),
      image_dimensions(std::move(other_img.image_dimensions)),
      image_size(std::move(other_img.image_size)) {}

CairoStb::CairoStb(const unsigned char *img_data, const size_type img_size) { load_image(img_data, img_size); }

CairoStb::CairoStb(const std::vector<unsigned char> &img_data) {
    load_image(img_data.data(), static_cast<size_type>(img_data.size()));
}

CairoStb &
CairoStb::operator=(CairoStb &&other_img) noexcept {
    std::swap(cairo_surface, other_img.cairo_surface);
    std::swap(image_dimensions, other_img.image_dimensions);
    std::swap(image_size, other_img.image_size);
    return *this;
}

CairoStb &
CairoStb::operator=(const CairoStb &other_img) {
    if (this == &other_img || !other_img.cairo_surface) {
        return *this;
    }

    auto new_cairo_surface = cairo_image_surface_create(
        CAIRO_SURFACE_TYPE, other_img.image_dimensions.width, other_img.image_dimensions.height);

    if (cairo_surface_status(new_cairo_surface) != CAIRO_STATUS_SUCCESS) {
        throw std::runtime_error("Failed to create Cairo image surface");
    }

    // Don't forget to flush surfaces before accessing raw data
    cairo_surface_flush(new_cairo_surface);
    auto new_cairo_surface_data = cairo_image_surface_get_data(new_cairo_surface);
    cairo_surface_flush(other_img.cairo_surface);
    const auto old_cairo_surface_data = cairo_image_surface_get_data(other_img.cairo_surface);

    // Copy old Cairo surface data to new
    std::memcpy(new_cairo_surface_data, old_cairo_surface_data, static_cast<std::size_t>(other_img.image_size));

    // After dropping data in, tell Cairo that it needs to reread the data
    cairo_surface_mark_dirty(new_cairo_surface);

    free_image_memory();
    this->cairo_surface = new_cairo_surface;
    return *this;
}

CairoStb::operator cairo_surface_t *() const noexcept { return cairo_surface; }

void
CairoStb::load_image(const unsigned char *img_data, const size_type buf_size) {
    int channels{};

    auto raw_pixel_data = stbi_load_from_memory(
        img_data, buf_size, &image_dimensions.width, &image_dimensions.height, &channels, STBI_rgb_alpha);

    if (!raw_pixel_data) {
        const auto error_msg = "Failed to load image: " + std::string(stbi_failure_reason());
        throw std::runtime_error(error_msg);
    }

    this->image_size = image_dimensions.width * image_dimensions.height * IMAGE_BYTE_PIXEL_AMNT;

    create_cairo_compatible_surface(raw_pixel_data);
}

CairoStb::Dimensions
CairoStb::dimensions() const noexcept {
    return image_dimensions;
}

cairo_surface_t *
CairoStb::get_surf() const noexcept {
    return cairo_surface;
}

void
CairoStb::create_cairo_compatible_surface(const unsigned char *raw_pixel_data) {
    cairo_surface = cairo_image_surface_create(CAIRO_SURFACE_TYPE, image_dimensions.width, image_dimensions.height);

    if (cairo_surface_status(cairo_surface) != CAIRO_STATUS_SUCCESS) {
        throw std::runtime_error("Failed to create Cairo image surface");
    }

    cairo_surface_flush(cairo_surface);
    auto surface_data = cairo_image_surface_get_data(cairo_surface);

    // STB output data is like:
    //  [0] = R
    //  [1] = G
    //  [2] = B
    //  [3] = A
    // Cairo expects:
    //  [0] = B
    //  [1] = G
    //  [2] = R
    //  [3] = A

    const auto img_stride = image_dimensions.width * IMAGE_BYTE_PIXEL_AMNT;
    auto pixel_data_pos = raw_pixel_data;

    // Each byte is either an r, g, b, or a value, making one row of the
    // image four times its width (a single byte for each value).
    //
    // We told STBI earlier that we expected the image to be rgba, even if it
    // isn't, so we can always assume there will be bytes for r, g, b, and a

    for (size_type height = 0; height < image_dimensions.height; ++height) {
        for (size_type width = 4; width < img_stride; width += 4) {
            auto r = pixel_data_pos[width - 4];
            auto g = pixel_data_pos[width - 3];
            auto b = pixel_data_pos[width - 2];
            auto a = pixel_data_pos[width - 1];

            // Alpha stored in upper 8 bits
            surface_data[width - 4] = b;
            surface_data[width - 3] = g;
            surface_data[width - 2] = r;
            surface_data[width - 1] = a;
        }

        surface_data += img_stride;
        pixel_data_pos += img_stride;
    }

    cairo_surface_mark_dirty(cairo_surface);

    stbi_image_free(static_cast<void *>(const_cast<unsigned char *>(raw_pixel_data)));
    raw_pixel_data = nullptr;
}

void
CairoStb::free_image_memory() noexcept {
    if (cairo_surface) {
        cairo_surface_destroy(cairo_surface);
        cairo_surface = nullptr;
    }
}

CairoStb::~CairoStb() { free_image_memory(); }
