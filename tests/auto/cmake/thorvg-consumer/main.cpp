// SPDX-License-Identifier: Apache-2.0
#include <thorvg.h>
#ifdef QCAT_TEST_QRHI
#include <qcatthorvgqrhicanvas.h>
#endif
#include <array>
#include <cstring>
#include <iostream>
#include <memory>

int main()
{
    uint32_t major = 0, minor = 0, micro = 0;
    tvg::Initializer::version(&major, &minor, &micro);
    if (major != 1 || minor != 1 || micro != 3)
        return 1;
    if (tvg::Initializer::init(0) != tvg::Result::Success)
        return 2;
    int result = 0;
    {
#ifdef QCAT_TEST_QRHI
        std::unique_ptr<QCatUiKit::ThorvgQrhiCanvas> qrhi(QCatUiKit::ThorvgQrhiCanvas::gen());
        if (!qrhi)
            return 5;
#endif
        std::array<uint32_t, 16 * 16> pixels{};
        std::unique_ptr<tvg::SwCanvas> canvas(tvg::SwCanvas::gen());
        auto picture = tvg::Picture::gen();
        constexpr char svg[] =
            "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"16\" height=\"16\">"
            "<rect width=\"16\" height=\"16\" fill=\"red\"/></svg>";
        if (!canvas || !picture ||
            canvas->target(pixels.data(), 16, 16, 16, tvg::ColorSpace::ARGB8888) != tvg::Result::Success ||
            picture->load(svg, std::strlen(svg), "svg") != tvg::Result::Success ||
            canvas->add(picture) != tvg::Result::Success ||
            canvas->draw(true) != tvg::Result::Success || canvas->sync() != tvg::Result::Success ||
            pixels[8 * 16 + 8] != 0xffff0000u)
            result = 3;
    }
    if (tvg::Initializer::term() != tvg::Result::Success)
        return 4;
    if (!result)
        std::cout << "ThorVG 1.1.3: SVG CPU rendering passed\n";
    return result;
}
