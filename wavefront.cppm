#pragma leco add_impl impl
export module wavefront;
import dotz;
import jute;
import hai;
import voo;

namespace wavefront {
  export struct vtx {
    dotz::vec3 pos;
    dotz::vec2 txt;
    dotz::vec3 nrm;
  };
  export hai::chain<wavefront::vtx> read_model(jute::view model);

  struct model_pair {
    voo::bound_buffer v_buffer;
    unsigned v_count;
  };
  export model_pair load_model(vee::physical_device pd, jute::view model);
}
