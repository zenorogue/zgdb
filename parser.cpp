#include <cmath>
#include <sstream>

// expression parser, adapted from HyperRogue

using ld = double;

struct parse_exception : std::exception {
  string s;
  const char *what() const noexcept override { return s.c_str(); }
  parse_exception(const string& z) : s(z) {}
  ~parse_exception() noexcept(true) {}
  };

map<string, unsigned> color_names = {
{"aliceblue", 0xf0f8ff}, {"antiquewhite", 0xfaebd7}, {"aqua", 0x00ffff}, {"aquamarine", 0x7fffd4}, {"azure", 0xf0ffff},
{"beige", 0xf5f5dc}, {"bisque", 0xffe4c4}, {"black", 0x000000}, {"blanchedalmond", 0xffebcd}, {"blue", 0x0000ff}, {"blueviolet", 0x8a2be2}, {"brown", 0xa52a2a}, {"burlywood", 0xdeb887},
{"cadetblue", 0x5f9ea0}, {"chartreuse", 0x7fff00}, {"chocolate", 0xd2691e}, {"coral", 0xff7f50}, {"cornflowerblue", 0x6495ed}, {"cornsilk", 0xfff8dc}, {"crimson", 0xdc143c}, {"cyan", 0x00ffff},
{"darkblue", 0x00008b}, {"darkcyan", 0x008b8b}, {"darkgoldenrod", 0xb8860b}, {"darkgray", 0xa9a9a9}, {"darkgreen", 0x006400}, {"darkkhaki", 0xbdb76b}, {"darkmagenta", 0x8b008b}, {"darkolivegreen", 0x556b2f},
{"darkorange", 0xff8c00}, {"darkorchid", 0x9932cc}, {"darkred", 0x8b0000}, {"darksalmon", 0xe9967a}, {"darkseagreen", 0x8fbc8f}, {"darkslateblue", 0x483d8b}, {"darkslategray", 0x2f4f4f}, {"darkturquoise", 0x00ced1},
{"darkviolet", 0x9400d3}, {"deeppink", 0xff1493}, {"deepskyblue", 0x00bfff}, {"dimgray", 0x696969}, {"dodgerblue", 0x1e90ff},
{"firebrick", 0xb22222}, {"floralwhite", 0xfffaf0}, {"forestgreen", 0x228b22}, {"fuchsia", 0xff00ff},
{"gainsboro", 0xdcdcdc}, {"ghostwhite", 0xf8f8ff}, {"gold", 0xffd700}, {"goldenrod", 0xdaa520}, {"gray", 0x808080}, {"green", 0x008000}, {"greenyellow", 0xadff2f},
{"honeydew", 0xf0fff0}, {"hotpink", 0xff69b4},
{"indianred ", 0xcd5c5c}, {"indigo", 0x4b0082}, {"ivory", 0xfffff0}, {"khaki", 0xf0e68c},
{"lavender", 0xe6e6fa}, {"lavenderblush", 0xfff0f5}, {"lawngreen", 0x7cfc00}, {"lemonchiffon", 0xfffacd}, {"lightblue", 0xadd8e6}, {"lightcoral", 0xf08080}, {"lightcyan", 0xe0ffff}, {"lightgoldenrodyellow", 0xfafad2},
{"lightgrey", 0xd3d3d3}, {"lightgreen", 0x90ee90}, {"lightpink", 0xffb6c1}, {"lightsalmon", 0xffa07a}, {"lightseagreen", 0x20b2aa}, {"lightskyblue", 0x87cefa}, {"lightslategray", 0x778899}, {"lightsteelblue", 0xb0c4de},
{"lightyellow", 0xffffe0}, {"lime", 0x00ff00}, {"limegreen", 0x32cd32}, {"linen", 0xfaf0e6},
{"magenta", 0xff00ff}, {"maroon", 0x800000}, {"mediumaquamarine", 0x66cdaa}, {"mediumblue", 0x0000cd}, {"mediumorchid", 0xba55d3}, {"mediumpurple", 0x9370d8}, {"mediumseagreen", 0x3cb371}, {"mediumslateblue", 0x7b68ee},
{"mediumspringgreen", 0x00fa9a}, {"mediumturquoise", 0x48d1cc}, {"mediumvioletred", 0xc71585}, {"midnightblue", 0x191970}, {"mintcream", 0xf5fffa}, {"mistyrose", 0xffe4e1}, {"moccasin", 0xffe4b5},
{"navajowhite", 0xffdead}, {"navy", 0x000080},
{"oldlace", 0xfdf5e6}, {"olive", 0x808000}, {"olivedrab", 0x6b8e23}, {"orange", 0xffa500}, {"orangered", 0xff4500}, {"orchid", 0xda70d6},
{"palegoldenrod", 0xeee8aa}, {"palegreen", 0x98fb98}, {"paleturquoise", 0xafeeee}, {"palevioletred", 0xd87093}, {"papayawhip", 0xffefd5}, {"peachpuff", 0xffdab9}, {"peru", 0xcd853f}, {"pink", 0xffc0cb}, {"plum", 0xdda0dd}, {"powderblue", 0xb0e0e6}, {"purple", 0x800080},
{"rebeccapurple", 0x663399}, {"red", 0xff0000}, {"rosybrown", 0xbc8f8f}, {"royalblue", 0x4169e1},
{"saddlebrown", 0x8b4513}, {"salmon", 0xfa8072}, {"sandybrown", 0xf4a460}, {"seagreen", 0x2e8b57}, {"seashell", 0xfff5ee}, {"sienna", 0xa0522d}, {"silver", 0xc0c0c0}, {"skyblue", 0x87ceeb}, {"slateblue", 0x6a5acd}, {"slategray", 0x708090}, {"snow", 0xfffafa}, {"springgreen", 0x00ff7f}, {"steelblue", 0x4682b4},
{"tan", 0xd2b48c}, {"teal", 0x008080}, {"thistle", 0xd8bfd8}, {"tomato", 0xff6347}, {"turquoise", 0x40e0d0},
{"violet", 0xee82ee},
{"wheat", 0xf5deb3}, {"white", 0xffffff}, {"whitesmoke", 0xf5f5f5},
{"yellow", 0xffff00}, {"yellowgreen", 0x9acd32}
};

struct likedata {
  int endpar;
  map<string, map<ld, ld>> qty;
  likedata() { endpar = 0; }
  };

extern stringstream find_log;

struct exp_parser {
  string s;
  int at;
  int line_number, last_line;
  game *g;
  exp_parser() { at = 0; line_number = 1; last_line = 0; }

  map<int, likedata> likedatas;

  ld get_like() {
    auto& lid = likedatas[at];
    if(!lid.endpar) {
      auto gbak = g;
      auto atbak = at;
      for(auto& h: games) {
        g = h;
        ld filter = parsepar();
        lid.endpar = at;
        at = atbak;
        if(filter)
          for(auto [t, val]: h->values)
            if(which_group[t] != "subjective" && which_group[t] != "metadata" && t != "random")
              lid.qty[t][val] += filter;
        }
      g = gbak;
      }

    at = lid.endpar;
    ld res = 0;
    for(auto& [t, subt]: lid.qty) {
      if(!g->values.count(t)) continue;
      ld myval = g->values[t];
      for(auto& [theirval, weight]: subt) {
        res += myval * theirval * weight;
        }
      }
    return res;
    }
  
  string where() { 
    if(s.find('\n')) return "(line " + its(line_number) + ", pos " + its(at-last_line) + ")";
    else return "(pos " + its(at) + ")";
    }
  
  bool ok() { return at == s.size(); }
  char next(int step=0) { if(at >= s.size()-step) return 0; else return s[at+step]; }
  
  char eatchar() {
    return s[at++];
    }

  bool eat(const char *c) {
    int orig_at = at;
    while(*c && *c == next()) at++, c++;
    if(*c == 0) return true;
    else at = orig_at;
    return false;
    }

  bool eat_token(const char *c) {
    int orig_at = at;
    if(next_token() == c) return true;
    at = orig_at;
    return false;
    }

  void skip_white();

  string next_token();

  char snext(int step=0) { skip_white(); return next(step); }

  vector<pair<ld, ld>> parse_with_reps();

  ld parse(int prio = 0);

  ld parsepar() {
    ld res = parse();
    force_eat(")");
    return res;
    }

  void force_eat(const char *c) {
    skip_white();
    if(!eat(c)) throw parse_exception("expected: " + string(c) + " at " + where());
    }

  };

void exp_parser::skip_white() {
  while(next() == ' ' || next() == '\n' || next() == '\r' || next() == '\t') {
    if(next() == '\r') last_line++;
    if(next() == '\n') {
      line_number++, last_line = at;
      }
    at++;
    }
  }

string exp_parser::next_token() {
  skip_white();
  string token;
  while(true) {
    char c = next();
    if((c >= '0' && c <= '9') || (c == '.' && next(1) != '.') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == ':')
      token += c, at++;
    else break;
    }
  return token;
  }

bool textsearch(const string& haystack, const string& needle) {
  string actual_haystack = "";
  for(auto c: haystack)
    if(c >= 'a' && c <= 'z') actual_haystack += c;
    else if(c >= 'A' && c <= 'Z') actual_haystack += (c + 'a' - 'A');

  return actual_haystack.find(needle) != string::npos;
  }

bool game_by(game *g, string s) {
  for(auto& x: g->by) if(textsearch(x, s)) return true;
  return false;
  }

ld exp_parser::parse(int prio) {
  ld res;
  skip_white();
  if(eat("sin(")) res = sin(parsepar());
  else if(eat("cos(")) res = cos(parsepar());
  else if(eat("sinh(")) res = sinh(parsepar());
  else if(eat("cosh(")) res = cosh(parsepar());
  else if(eat("asin(")) res = asin(parsepar());
  else if(eat("acos(")) res = acos(parsepar());
  else if(eat("asinh(")) res = asinh(parsepar());
  else if(eat("acosh(")) res = acosh(parsepar());
  else if(eat("exp(")) res = exp(parsepar());
  else if(eat("sqrt(")) res = sqrt(parsepar());
  else if(eat("log(")) res = log(parsepar());
  else if(eat("tan(")) res = tan(parsepar());
  else if(eat("tanh(")) res = tanh(parsepar());
  else if(eat("atan(")) res = atan(parsepar());
  else if(eat("atanh(")) res = atanh(parsepar());
  else if(eat("abs(")) res = abs(parsepar());
  else if(eat("floor(")) res = floor(parsepar());
  // else if(eat("not(")) res = parsepar() > 0 ? 0 : 1;
  else if(eat("frac(")) { res = parsepar(); res = res - floor(res); }
  else if(eat("min(")) {
    ld a = parse(0);
    while(skip_white(), eat(",")) a = min(a, parse(0));
    force_eat(")");
    res = a;
    }
  else if(eat("max(")) {
    ld a = parse(0);
    while(skip_white(), eat(",")) a = max(a, parse(0));
    force_eat(")");
    res = a;
    }
  else if(eat("atan2(")) {
    ld y = parse(0);
    force_eat(",");
    ld x = parse(0);
    force_eat(")");
    res = atan2(y, x);
    }
  else if(eat("ifp(")) {
    ld cond = parse(0);
    force_eat(",");
    ld yes = parse(0);
    force_eat(",");
    ld no = parsepar();
    res = cond > 0 ? yes : no;
    }  
  else if(eat("ifz(")) {
    ld cond = parse(0);
    force_eat(",");
    ld yes = parse(0);
    force_eat(",");
    ld no = parsepar();
    res = abs(cond) < 1e-8 ? yes : no;
    }
  /* else if(eat("let(")) {
    string name = next_token();
    force_eat("=");
    ld val = parse(0);
    force_eat(",");
    dynamicval<cld> d(extra_params[name], val);
    res = parsepar();
    } */
  else if(eat("links(")) {
    int pos = at;
    auto gbak = g;
    res = 0; parse(0);
    for(auto link: g->links_to) {
      g = game_by_name[link];
      at = pos;
      res += parse(0);
      }
    g = gbak;
    force_eat(")");
    }
  else if(eat("like(")) res = get_like();
  else if(eat("linked(")) {
    int pos = at;
    auto gbak = g;
    res = 0; parse(0);
    for(auto link: g->linked_by) {
      g = game_by_name[link];
      at = pos;
      res += parse(0);
      }
    g = gbak;
    force_eat(")");
    }
  else if(next() == '(') at++, res = parsepar(); 
  else {
    string number = next_token();
    if(which_group.count(number) && g) res = g->values.count(number) ? g->values[number] : 0;
    else if(number.substr(0, 4) == "def:") res = g->values.count(number.substr(4));
    else if(number.substr(0, 8) == "unknown:") res = g->qtags.count(number.substr(8));
    else if(number.substr(0, 3) == "by:") res = game_by(g, number.substr(3));
    else if(number.substr(0, 6) == "title:") {
      string tit = number.substr(6);
      res = textsearch(g->title, tit);
      for(auto& x: g->other_titles) res += textsearch(x, tit);
      }
    else if(number.substr(0, 5) == "text:") {
      string tit = number.substr(5);
      res = textsearch(g->title, tit);
      for(auto& x: g->other_titles) res += textsearch(x, tit);
      for(auto& x: g->reviews) res += textsearch(x, tit);
      for(auto& [t, x]: g->tagcomments) res += textsearch(x, tit);
      }
    else if(tags_in_group.count(number) && g) {
      res = 0;
      for(auto v: tags_in_group[number]) res += g->values.count(v);
      }
    else if(number == "images") res = g->pngs.size();
    else if(number == "spoilers") res = g->spoilers.size();
    else if(number == "e") res = exp(1);
    else if(number == "inf") res = HUGE_VAL;
    else if(number == "p" || number == "pi") res = M_PI;
    else if(number == "tau") res = 2 * M_PI;
    else if(number == "phi") res = (1 + sqrt(5)) / 2;
    else if(number == "" && next() == '-') { at++; res = -parse(20); }
    else if(number == "") throw parse_exception("number missing, " + where());
    else if(number[0] == '0' && number[1] == 'x') res = strtoll(number.c_str()+2, NULL, 16);
    else if(number == "deg") res = M_PI / 180;
    else if(number == "not") res = parse(50) ? 0 : 1;
    else if(color_names.count(number)) res = color_names[number];
    else {
      if(number.back()== 'e' || number.back() == 'E') {
        if(eat("-")) number = number + "-" + next_token();
        else if(eat("+")) number = number + "+" + next_token();
        }
      std::stringstream ss; res = 0; ss << number;
      ss >> res;
      if(ss.fail() || !ss.eof()) throw parse_exception("unknown value: " + number);
      }
    }
  while(true) {
    skip_white();
         if(prio <= 3 && next() == '|') at++, res = max(res, parse(5));
    else if(prio <= 4  && eat_token("or")) { auto other = parse(5); if(res == 0) res = other; }
    else if(prio <= 5  && next() == '&') at++, res = min(res, parse(7));
    else if(prio <= 5  && eat_token("and")) { auto other = parse(7); if(res) res = other; }
    else if(prio <= 7  && eat(">=")) res = res >= parse(10);
    else if(prio <= 7  && eat("<=")) res = res <= parse(10);
    else if(prio <= 7  && eat("==")) res = res == parse(10);
    else if(prio <= 7  && eat("!=")) res = res != parse(10);
    else if(prio <= 7  && next() == '>') at++, res = res > parse(10);
    else if(prio <= 7  && next() == '<') at++, res = res < parse(10);
    else if(prio <= 10 && next() == '+') at++, res = res + parse(20);
    else if(prio <= 10 && next() == '-') at++, res = res - parse(20);
    else if(prio <= 20 && next() == '*') at++, res = res * parse(30);
    else if(prio <= 20 && next() == '/') at++, res = res / parse(30);
    else if(prio <= 40 && next() == '^') at++, res = pow(res, parse(40));
    else break;
    }
  return res;
  }

ld parseld(const string& s, game *g) {
  exp_parser ep;
  ep.s = s; ep.g = g;
  return ep.parse();
  }

