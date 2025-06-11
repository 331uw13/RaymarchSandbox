#ifndef UNIFORMS_TAB_HPP
#define UNIFORMS_TAB_HPP


class RMSB;
struct uniform_t;

namespace UniformsTab
{
    void edit_uniform(struct uniform_t* uniform);
    void render(RMSB* rmsb);
}


#endif
