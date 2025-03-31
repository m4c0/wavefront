#pragma leco app
#pragma leco add_shader "poc.vert"
#pragma leco add_shader "poc.frag"

import vee;
import voo;
import vapp;

struct lng : public vapp {
  void run() override {
    main_loop("poc-voo", [&](auto & dq, auto & sw) {
      auto pl = vee::create_pipeline_layout();
      voo::one_quad_render oqr { "poc", &dq, *pl };
      extent_loop(dq.queue(), sw, [&] {
        sw.queue_one_time_submit(dq.queue(), [&](auto pcb) {
          auto scb = sw.cmd_render_pass({ *pcb });
          oqr.run(*pcb, sw.extent());
        });
      });
    });
  }
} t;
