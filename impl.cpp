module wavefront;
import jojo;
import jute;
import hai;
import silog;
import sires;

struct invalid_number {};
static constexpr int atoi(jute::view v) {
  int res = 0;
  for (auto c : v) {
    if (c < '0' || c > '9') throw invalid_number {};
    res = res * 10 + (c - '0');
  }
  return res;
}
static constexpr float atof(jute::view v) {
  float res = 0;
  int decimals = -1;
  bool negative = v[0] == '-';
  if (negative) v = v.subview(1).after;
  for (auto c : v) {
    if (c == '.' && decimals == -1) {
      decimals = 0;
      continue;
    } else if (c == '.') throw invalid_number {};
    if (c < '0' || c > '9') throw invalid_number {};

    res = res * 10 + (c - '0');

    if (decimals >= 0) decimals++;
  }
  for (auto i = 0; i < decimals; i++) res /= 10;
  return negative ? -res : res;
}

static wavefront::vtx read_vertex(auto & pos, auto & txt, auto & nrm, jute::view vtn) {
  auto [v, vr] = vtn.split('/');
  auto [t, tr] = vr.split('/');
  auto [n, nr] = tr.split('/');
  
  return wavefront::vtx {
    .pos = pos.seek(atoi(v) - 1),
    .txt = txt.seek(atoi(t) - 1),
    .nrm = nrm.seek(atoi(n) - 1),
  };
}
hai::chain<wavefront::vtx> wavefront::read_model(jute::view model) {
  hai::chain<dotz::vec3> pos { 10240 };
  hai::chain<dotz::vec2> txt { 10240 };
  hai::chain<dotz::vec3> nrm { 10240 };
  hai::chain<wavefront::vtx> vtx { 102400 };

  jojo::readlines(sires::real_path_name(model), [&](auto line) {
    if (line.size() < 2) return;
    auto [cmd, vr] = line.split(' ');
    auto [x, xr] = vr.split(' ');
    auto [y, yr] = xr.split(' ');
    if (cmd == "v") {
      auto [z, zr] = yr.split(' ');
      dotz::vec3 p { atof(x), atof(y), atof(z) };
      pos.push_back(p);
    } else if (cmd == "vn") {
      auto [z, zr] = yr.split(' ');
      dotz::vec3 n { atof(x), atof(y), atof(z) };
      nrm.push_back(n);
    } else if (cmd == "vt") {
      txt.push_back({ atof(x), atof(y) });
    } else if (cmd == "f") {
      auto [z, zr] = yr.split(' ');
      auto [w, wr] = zr.split(' ');
      auto v0 = read_vertex(pos, txt, nrm, x);
      auto v1 = read_vertex(pos, txt, nrm, y);
      auto v2 = read_vertex(pos, txt, nrm, z);
      vtx.push_back(v0);
      vtx.push_back(v1);
      vtx.push_back(v2);
      if (!w.size()) return;
      auto v3 = read_vertex(pos, txt, nrm, w);
      vtx.push_back(v0);
      vtx.push_back(v2);
      vtx.push_back(v3);
    }
  });

  return vtx;
}

static void load_model(voo::h2l_buffer & buf, auto & vs) {
  voo::mapmem mm { buf.host_memory() };
  auto * m = static_cast<wavefront::vtx *>(*mm);
  for (auto v : vs) *m++ = v;
}

wavefront::model_pair wavefront::load_model(vee::physical_device pd, jute::view model) {
  struct model_pair res {};
  auto vs = wavefront::read_model(model);

  unsigned v_count = res.v_count = vs.size();
  unsigned v_size = v_count * sizeof(wavefront::vtx);
  res.v_buffer = voo::h2l_buffer { pd, v_size };
  ::load_model(res.v_buffer, vs);
  silog::log(silog::info, "Loaded model %.*s", static_cast<unsigned>(model.size()), model.begin());
  return res;
}


