//
// Created by lenovo on 3/27/2024.
//

#ifndef VKMMD_ALIGNMENT_H
#define VKMMD_ALIGNMENT_H

namespace vkH {

    template<class integral>
    constexpr integral align_up(integral x, size_t a) noexcept{
        return integral((x + (integral(a) - 1)) & ~integral(a - 1));
    }

} // vkHelp

#endif //VKMMD_ALIGNMENT_H
