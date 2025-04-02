#pragma leco add_impl impl
export module wavefront;
import dotz;
import jute;
import hai;

namespace wavefront {
  export struct vtx {
    dotz::vec3 pos;
    dotz::vec2 txt;
    dotz::vec3 nrm;
  };
  export hai::chain<wavefront::vtx> read_model(jute::view model);
}
