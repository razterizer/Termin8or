#pragma once
#include "Screen.h"
#include "RC.h"
#include "Gradient.h"
#include <Core/Rand.h>
#include <Core/Math.h>

struct Particle
{
  Particle() = default;
  
  RC start_pos;
  float time_stamp = 0.f;
  float life_time = 2.f;
  float vel_r = 0.f;
  float vel_c = 0.f;
  float pos_r = 0.f;
  float pos_c = 0.f;
  float g = -30.f;
  bool dead = false;
  
  void init(float time, const RC& pos0, float vr, float vc, float gravity_acc, float spread, float life_t)
  {
    time_stamp = time;
    start_pos = pos0;
    vel_r = vr + spread * (rnd::rand() - 0.5f);
    vel_c = vc + spread * (rnd::rand() - 0.5f);
    g = gravity_acc;
    pos_r = static_cast<float>(pos0.r);
    pos_c = static_cast<float>(pos0.c);
    life_time = life_t;
  }
  
  void update(float dt)
  {
    pos_c += vel_c * dt;
    pos_r += vel_r * dt / pix_ar;
    pos_r += g * dt; // r is pointing down.
  }
  
  bool alive(float time) const
  {
    return time - time_stamp < life_time;
  }
  
  template<int NR, int NC>
  void draw(SpriteHandler<NR, NC>& sh, const std::string& str, Color fg_color, Color bg_color, float time) const
  {
    if (alive(time))
      sh.write_buffer(str, std::round(pos_r), std::round(pos_c), fg_color, bg_color);
  }
};

struct ParticleGradientGroup
{
  Gradient<Color> fg_color_gradient;
  Gradient<Color> bg_color_gradient;
  Gradient<std::string> string_gradient;
};

struct ParticleHandler
{
  ParticleHandler(size_t N_particles)
    : particle_stream(N_particles), num_particles(N_particles), num_particles_active(N_particles) {}
  
  void update(const RC& start_pos, bool trigger,
              float vel_r, float vel_c, float g,
              float spread, float life_time, int particle_cluster_size,
              float dt, float time)
  {
    int particle_cluster_idx = 0;
    for (auto& particle : particle_stream)
    {
      if (particle.alive(time))
        particle.update(dt);
      else if (trigger)
      {
        if (!particle.dead)
          particle.init(time, start_pos, vel_r, vel_c, g, spread, life_time);
        if (particle_cluster_idx++ >= particle_cluster_size)
          trigger = false;
      }
    }
  }
  
  template<int NR, int NC>
  void draw(SpriteHandler<NR, NC>& sh, const std::string& str, Color fg_color, Color bg_color, float time) const
  {
    for (const auto& particle : particle_stream)
      if (!particle.dead)
        particle.draw(sh, str, fg_color, bg_color, time);
  }
  
  template<int NR, int NC>
  void draw(SpriteHandler<NR, NC>& sh, const std::vector<std::string>& str,
    const Gradient<Color>& fg_color, const Gradient<Color>& bg_color, float time) const
  {
    for (const auto& particle : particle_stream)
      if (!particle.dead && particle.alive(time))
      {
        auto t = (time - particle.time_stamp)/particle.life_time;
        int str_idx = static_cast<int>(std::round(t*str.size())) - 1;
        str_idx = math::clamp(str_idx, 0, static_cast<int>(str.size()) - 1);
        particle.draw(sh, str[str_idx], fg_color(t), bg_color(t), time);
      }
  }
  
  template<int NR, int NC>
  void draw(SpriteHandler<NR, NC>& sh, const std::vector<std::string>& str,
    const std::vector<std::pair<float, std::pair<Gradient<Color>, Gradient<Color>>>>& color_fg_bg_vec,
    float time) const
  {
    for (const auto& particle : particle_stream)
      if (!particle.dead && particle.alive(time))
      {
        auto t = (time - particle.time_stamp)/particle.life_time;
        int str_idx = static_cast<int>(std::round(t*str.size())) - 1;
        str_idx = math::clamp(str_idx, 0, static_cast<int>(str.size()) - 1);
        const auto& col_fg_bg = rnd::rand_select(color_fg_bg_vec);
        const Gradient<Color>& fg_color = col_fg_bg.first;
        const Gradient<Color>& bg_color = col_fg_bg.second;
        particle.draw(sh, str[str_idx], fg_color(t), bg_color(t), time);
      }
  }
  
  template<int NR, int NC>
  void draw(SpriteHandler<NR, NC>& sh,
    const std::vector<std::pair<float, ParticleGradientGroup>>& gradient_groups,
    float time) const
  {
    for (const auto& particle : particle_stream)
      if (!particle.dead && particle.alive(time))
      {
        auto t = (time - particle.time_stamp)/particle.life_time;
        const auto& grads = rnd::rand_select(gradient_groups);
        const Gradient<Color>& fg_grad = grads.fg_color_gradient;
        const Gradient<Color>& bg_grad = grads.bg_color_gradient;
        const Gradient<std::string>& str_grad = grads.string_gradient;
        particle.draw(sh, str_grad(t), fg_grad(t), bg_grad(t), time);
      }
  }
  
  void set_num_active_particles(float amount_ratio_active)
  {
    num_particles_active = static_cast<int>(std::round(static_cast<float>(num_particles) * amount_ratio_active));
    for (size_t p_idx = num_particles_active; p_idx < num_particles; ++p_idx)
      particle_stream[p_idx].dead = true;
  }
  
  std::vector<Particle> particle_stream;
  const size_t num_particles = 0;
  
private:
  size_t num_particles_active = 0;
};
