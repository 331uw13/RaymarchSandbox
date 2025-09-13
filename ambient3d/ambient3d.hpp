#ifndef AMBIENT3D_HPP
#define AMBIENT3D_HPP

#include "raylib.h"
#include <cstdint>
#include <map>
#include <array>
#include <functional>

#include "player.hpp"
#include "terrain.hpp"
#include "shader_util.hpp"
#include "uniform_buffer.hpp"
#include "light.hpp"
#include "renderable.hpp"
#include "glsl_preproc.hpp"
#include "util.hpp"
#include "network/network.hpp"


namespace AM {

    static constexpr int NUM_BLOOM_SAMPLES = 16;
    static constexpr int MAX_LIGHTS = 64;

    static constexpr int CHAT_KEY = KEY_ENTER;


    enum ShaderIDX : int {
        DEFAULT,
        DEFAULT_INSTANCED,
        POST_PROCESSING,
        BLOOM_TRESHOLD,
        BLOOM_DOWNSAMPLE_FILTER,
        BLOOM_UPSAMPLE_FILTER,
        // ...
    };

    enum GuiModuleFocus {
        GAIN,
        LOSE,
        TOGGLE
    };

    class State {
        public:
            State(uint16_t win_width, uint16_t win_height,
                    const char* title,  AM::NetConnectCFG network_cfg);

            ~State();

            Font     font;
            Player   player;
            Terrain  terrain;

            Network* net;


            void    frame_begin();
            void    frame_end();

            // Shaders in this array will be unloaded when the state is destructed.
            std::vector<Shader> shaders;
            void add_shader(const Shader& shader);

            Light** add_light(const Light& light);
            void    remove_light(Light** light);
            void    update_lights();
            void    draw_info();
            void    draw_text(int font_size, const char* text, int x, int y, const Color& color);

            template<class MODULE>
            void register_gui_module(
                    AM::GuiModuleID module_id, // User chosen ID.
                    AM::GuiModule::RenderOPT render_option
            ){
                m_gui_modules.push_back(
                        std::make_unique<MODULE>(MODULE(module_id, render_option)));
            }

            template<class MODULE>
            MODULE* find_gui_module(AM::GuiModuleID module_id) {
                for(size_t i = 0; i < m_gui_modules.size(); i++) {
                    if(m_gui_modules[i]->get_id() == module_id) {
                        return dynamic_cast<MODULE*>(m_gui_modules[i].get());
                    }
                }
                return NULL;
            }

            void    set_gui_module_focus(int module_id, GuiModuleFocus focus_option);

            // When amount is close to 0.0 small distortions happen
            // but when it reaches 0.5 "blinking" starts happening
            // and it gets stronger towards 1.0
            void    set_vision_effect(float amount);


            void    set_mouse_enabled(bool enabled);
            bool    is_mouse_enabled() { return m_mouse_enabled; }

            void    set_movement_enabled(bool enabled) { m_movement_enabled = enabled; }
            bool    is_movement_enabled() { return m_movement_enabled; }


            void set_fixed_tick_callback(std::function<void(AM::State*)> callback) {
                m_fixed_tick_callback = callback;
                m_fixed_tick_callback_set = true;
            }
            void set_fixed_tick_speed(float tick_speed) {
                m_fixed_tick_speed = tick_speed;
            }

        private:
            bool m_mouse_enabled          { true };
            bool m_movement_enabled       { true };
            bool m_connected_to_server    { false };
            bool m_is_chat_open           { false };

            asio::io_context m_asio_io_context;

            enum RenderTargetIDX : int {
                RESULT,
                BLOOM_TRESHOLD,
                BLOOM_PRE_RESULT,
                BLOOM_RESULT,

                NUM_TARGETS
            };

            std::array<RenderTexture2D, RenderTargetIDX::NUM_TARGETS>
                m_render_targets;

            std::array<RenderTexture2D, NUM_BLOOM_SAMPLES>
                m_bloom_samples;

            ItemManager                      m_item_manager;
            void                             m_render_dropped_items();

            bool                             m_fixed_tick_callback_set  { false };
            float                            m_fixed_tick_timer         { 0.0f };
            float                            m_fixed_tick_speed         { 0.075f };
            std::function<void(AM::State*)>  m_fixed_tick_callback;
            void                             m_fixed_tick_internal();
            void                             m_update_gui_module_inputs();

            // TODO Maybe move this away from State class
            void m_render_bloom();

        

            UniformBuffer m_lights_ubo;
            std::array<Light, MAX_LIGHTS> m_lights;
            std::array<Light*, MAX_LIGHTS> m_light_ptrs { NULL };
            size_t m_num_lights { 0 };

            int64_t                                 m_focused_gui_module_idx { -1 };
            std::vector<std::unique_ptr<GuiModule>> m_gui_modules;
            std::map<int/*light ID*/, Light>        m_lights_pframe_map; // Previous frame lights.
           


    };

};




#endif
