// to do:
// - fix todos
// - info about more by's?
// - info about more philosophies
// - provide permanent links for searches

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include "visutils.h"
#include <cstdlib>
#include <cstdio>

#ifdef ITCH
constexpr bool large_font = false;
#else
constexpr bool large_font = true;
#endif

using namespace std;
bool is_mobile;

string showing_what;

int changes;

void do_main_page();

vector<string> parselog;

void plog(string s) { parselog.push_back(s); }

#include "readdata.cpp"

#include "parser.cpp"

void add_button(ostream& of, string action, string text) {
  of << "<button type='button' onclick=\"" << action << "\"/>" << text << "</button>";
  }

void out_tag_value(ostream& of, game *g, const string &tag) {
  if(!is_numeric[tag] && (g->values[tag] == 0 || g->values[tag] == 1))
     of << (g->values[tag] ? "✅" : "❌");
  else of << "=" << g->values[tag];
  if(g->qtags.count(tag)) of << "<font color=\"FFC0C0\"> (not sure)</font>";
  }

void out_desc(ostream& of, const string& s) {
  bool in_link = false;
  string link = "";
  for(char c: s) {
    if(!in_link) {
      if(c != '{') of << c;
      else in_link = true, link = "";
      }
    else if(in_link) {
      if(c != '}') link += c;
      else {
        if(link.substr(0, 4) == "tag:") {
          auto tg = link.substr(4);
          of << "<a onClick=\"explain_tag('" << tg << "')\">" << tg << "</a>";
          }
        else if(link.substr(0, 6) == "group:") {
          auto gr = link.substr(6);
          of << "<a onClick=\"explain_group('" << gr << "')\">" << gr << "</a>";
          }
        else if(link.substr(0, 3) == "by:") {
          auto by = link.substr(3);
          of << "<a onClick=\"explain_by('" << by << "')\">" << by << "</a>";
          }
        else {
          of << "<a onClick=\"explain_game('" << link << "')\">" << link << "</a>";
          }
        in_link = false;
        }
      }
    }
  }

bool is_by(game* g, const string& s) {
  for(auto& b: g->by) if(b == s) return true;
  return false;
  }

bool comments_on(game* g, const string& s) {
  return false;
  }

void display(ostream& of, game *g) {
  if(!is_mobile) {
    of << "<div style=\"float:left;width:100%\">\n";
    of << "<div style=\"float:left;width:10%\">&nbsp;</div>";
    of << "<div style=\"float:left;width:" << (g->pngs.size() ? "40" : "80") << "%\">";
    of << "<br/>";
    }
  of << "<h1>" << g->title << "</h1><br/>";

  for(auto &aka: g->other_titles) {
    of << "<b><font color=\"80FF80\">aka " << aka << "</font></b><br/>";
    }

  for(auto gr: all_groups) {
    bool has_group = false;
    for(auto tag: tags_in_group[gr]) if(g->values.count(tag)) has_group = true;
    if(!has_group) continue;

    of << "<b><a onClick=\"explain_group('" << gr << "')\">" << gr << "</a></b>: ";

    for(auto& tag: tags_in_group[gr]) if(g->values.count(tag)) {
      of << "<b><a onClick=\"explain_tag('" << tag << "')\">" << tag << "</a></b>";
      out_tag_value(of, g, tag); of << " ";
      }
    of << "<br/>\n";
    }

  if(g->by.size()) of << "<br/>";
  for(auto& b: g->by) {
    of << "<b><font color=\"FF80FF\">By:</font></b> <a onClick=\"explain_by('" << b << "')\">" << b << "</a><br/>";
    }

  of << "<br/>";
  for(auto &ff: g->reviews) {
    of << "<b><font color=\"80FF80\">Review:</font></b><br/>";
    out_desc(of, ff);
    of << "<br/><br/>";
    }
  of << "<br/>";

  for(auto &lby: g->linked_by) {
    of << "<b><font color=\"C0FFC0\">linked by <a onClick=\"explain_game('" << lby << "')\">" << lby << "</a></font></b><br/>";
    }
  if(g->linked_by.size()) of << "<br/>";

  if(is_mobile) {
    for(auto &png: g->pngs) {
      of << "<table><tbody><tr>";
      of << "<TD ALIGN=CENTER VALIGN=BOTTOM>";
      of << "<A target=\"_blank\" href=\"pngs/" + png.first + ".png\" style=\"text-decoration:none\"><IMG src=\"pngs/" + png.first + ".png\" width=\"90%\" BORDER=\"2\" class=\"noborder\"/>";
      of << "<div style=\"text-align: center\">" + png.second + "</div></A></TD></tr></tbody></table>";
      of << "<br/>";
      }
    }

  if(!is_mobile && !g->pngs.empty()) {
    of << "</div>\n";
    of << "<div style=\"float:left;width:40%\">";
    for(auto &png: g->pngs) {
      of << "<table><tr>";
      of << "<TD ALIGN=CENTER VALIGN=BOTTOM>";
      of << "<A target=\"_blank\" href=\"pngs/" + png.first + ".png\" style=\"text-decoration:none\"><IMG src=\"pngs/" + png.first + ".png\" width=\"90%\" style=\"border: 5x solid %55;\"/>";
      of << "<div style=\"text-align: center\">" + png.second + "</div></A></TD></tr></table>";
      of << "<br/>";
      }
    }
 
  if(!is_mobile) {
    of << "</div>\n";
    of << "</div>\n";
    }
  }

void add_buttons(ostream& ss) {
  ss << "<center>";
  ss << "<a onclick=\"main_page()\">main page</a>";
  ss << " | <a onclick=\"explain_by('zenorogue')\">coi</a>";
  ss << " | <a onclick=\"explain_group('subjective')\">subjective</a>";
  ss << "</center>";
  ss << "<br/><br/>";
  }

string mainpart, param;
char which;

EM_JS(void, push_state, (const char *mp, const char* which, const char* param), {
  history.pushState({}, null, UTF8ToString(mp) + "?" + UTF8ToString(which) + "=" + encodeURIComponent(UTF8ToString(param)));
  });

EM_JS(void, push_state_empty, (const char *mp), {
  history.pushState({}, null, UTF8ToString(mp));
  });

EM_JS(const char*, decodeURI, (const char* uri), {
  return stringToNewUTF8(decodeURIComponent(UTF8ToString(uri)));
  });

void push_state_if_changed(char _which, string _param) {
  if(which != _which || param != _param) {
    which = _which; param = _param;
    string xwhich; xwhich += which;
    push_state(mainpart.c_str(), xwhich.c_str(), param.c_str());
    }
  }

void link_error() {
  stringstream ss;
  ss << "error";
  ss << "<hr/>\n";
  add_buttons(ss);
  set_value("output", ss.str());
  }

void do_explain_group(const string& g) {
  if(!groupdesc.count(g)) return link_error();
  stringstream ss;
  ss << "<div style=\"float:left;width:100%\">\n";
  ss << "<div style=\"float:left;width:10%\">&nbsp;</div>";
  ss << "<div style=\"float:left;width:80%\">";
  ss << "<h1>" << g << "</h1>";
  out_desc(ss, groupdesc[g]);
  ss << "<br/><br/>";
  for(auto tag: tags_in_group[g]) {
    ss << "<b><font color=\"80FF80\">" 
    << "<a onClick=\"explain_tag('" << tag << "')\">" << tag << "</a>"
    << ":</font></b><br/>";
    out_desc(ss, tagdesc[tag]);
    ss << "<br/><br/>";
    }
  ss << "</div></div>\n";
  ss << "<hr/>\n";
  add_buttons(ss);
  set_value("output", ss.str());
  push_state_if_changed('x', g);
  }


void do_explain_tag(const string& s0) {
  auto s = s0;
  if(s == "philosophies") s = "philosophy";
  if(!which_group.count(s)) return link_error();
  stringstream ss;
  ss << "<div style=\"float:left;width:100%\">\n";
  ss << "<div style=\"float:left;width:10%\">&nbsp;</div>";
  ss << "<div style=\"float:left;width:80%\">";
  ss << "<h1>" << s0 << "</h1>";
  string g = which_group[s];
  ss << "<h2>A tag from group " 
     << "<a onClick=\"explain_group('" << g << "')\">" << g << "</a></h2>"
     << "<br/>";
  out_desc(ss, tagdesc[s]);
  ss << "<br/><br/>";
  vector<pair<double, string> > info;
  for(auto& g: games) 
    if(g->values.count(s)) info.emplace_back(-g->values[s], g->title);
    else if(comments_on(g, s)) info.emplace_back(0, g->title);
    else if(g->qtags.count(s)) info.emplace_back(99999, g->title);

  sort(info.begin(), info.end());

  ss << "<ul>";
  for(auto& [val, t]: info) {
    ss << "<li>";
    auto g = game_by_name[t];
    ss << "<a onClick=\"explain_game('" << t << "')\">" << t << "</a>";
    if(g->values.count(s)) out_tag_value(ss, g, s);
    ss << "</li>";
    }
  ss << "</ul>";
  ss << "</div></div>\n";
  ss << "<hr/>\n";
  add_buttons(ss);
  set_value("output", ss.str());
  push_state_if_changed('t', s);
  }

void do_explain_by(const string& s) {
  if(!all_devs.count(s)) return link_error();
  stringstream ss;
  ss << "<div style=\"float:left;width:100%\">\n";
  ss << "<div style=\"float:left;width:10%\">&nbsp;</div>";
  ss << "<div style=\"float:left;width:80%\">";
  if(about_dev.count(s)) {
    ss << "<h1>" << "about " << s << "</h1>";
    out_desc(ss, about_dev[s]);
    ss << "<h2>" << "games by " << s << "</h1>";
    }
  else ss << "<h1>" << "games by " << s << "</h1>";
  ss << "<ul>";
  for(auto& g: games) if(is_by(g, s))
    ss << "<li><a onClick=\"explain_game('" << g->title << "')\">" << g->title << "</a></li>";
  ss << "</ul>";
  ss << "<h2>other developers</h2><li>";
  for(auto& [s, q]: all_devs) ss << "<a onClick=\"explain_by('" << s << "')\">" << s << "</a>: " << q << (about_dev.count(s) ? "+" : "") << " ";
  ss << "</li><hr/>\n";
  add_buttons(ss);
  set_value("output", ss.str());
  push_state_if_changed('b', s);
  }

void do_explain_game(const string &s) {
  if(!game_by_name.count(s)) return link_error();
  stringstream ss;
  ss << "<div style=\"float:left;width:100%\">\n";
  ss << "<div style=\"float:left;width:10%\">&nbsp;</div>";
  ss << "<div style=\"float:left;width:80%\">";
  display(ss, game_by_name[s]);
  ss << "<hr/>\n";
  add_buttons(ss);
  set_value("output", ss.str());
  push_state_if_changed('g', s);
  }

void do_main_page() {
  stringstream ss;

  ss << "<div style=\"float:left;width:100%\">\n";
  ss << "<div style=\"float:left;width:10%\">&nbsp;</div>";
  ss << "<div style=\"float:left;width:80%\">";

  out_desc(ss, about);
  ss << "<br/><br/>";

  ss << "Filter:<br/><input id=\"filter\" size=100 type=text onInput=\"on_find_change(document.getElementById('filter').value, document.getElementById('sorting').value, document.getElementById('coloring').value)\"><br/><br/>";

  ss << "Sorting:<br/><input id=\"sorting\" size=100 type=text  onInput=\"on_find_change(document.getElementById('filter').value, document.getElementById('sorting').value, document.getElementById('coloring').value)\"/><br/><br/>";

  ss << "Coloring:<br/><input id=\"coloring\" size=100 type=text  onInput=\"on_find_change(document.getElementById('filter').value, document.getElementById('sorting').value, document.getElementById('coloring').value)\"/><br/><br/>";

  add_button(ss, "find_games()", "list games!");
  ss << "<br/><br/>\n";

  ss << "<div id=gamelist></div>";

  ss << "</div></div>";

  ss << "<hr/>\n";

  for(auto z: parselog) ss << z << "<br/>";
  add_buttons(ss);
  set_value("output", ss.str());
  if(which != 'z') {
    which = 'z';
    push_state_empty(mainpart.c_str());
    }
  }

vector<game*> found;
stringstream find_log;

void game_search(const string& filter, const string& sorting, const string& coloring) {
  found.clear();
  std::stringstream().swap(find_log);

  exp_parser epf; epf.s = filter;
  exp_parser eps; eps.s = sorting;
  exp_parser epc; epc.s = coloring;

  for(auto& g: games) {
    epf.g = eps.g = epc.g = g;
    epf.at = eps.at = epc.at = 0;
    g->current_random_value = 1 + rand() & 0xFFFFFF;
    g->values["random"] = g->current_random_value;
    try {
      auto val = (filter == "") ? 1 : epf.parse();
      if(val > 0) {
        g->current_sort_value = (sorting == "") ? 1 : eps.parse();
        g->current_color_value = (coloring == "") ? 0xFFFFFF : epc.parse();
        found.push_back(g);
        }
      }
    catch(parse_exception& e) {
      find_log << "exception: " << e.what() << " (while evaluating " << g->title << ")<br/>";
      found.clear();
      return;
      }
    }
  sort(found.begin(), found.end(), [] (game *a, game *b) { return tie(a->current_sort_value, a->current_color_value, a->current_random_value) > tie(b->current_sort_value, b->current_color_value, b->current_random_value); });
  }

int init(bool _is_mobile) {
  is_mobile = _is_mobile;
  srand(time(NULL));

  printf("Parsing...\n");
  parse("zgdb.txt");
  for(auto g: games) for(auto l: g->links_to) 
    game_by_name[l]->linked_by.insert(g->title);

  string href = (char*)EM_ASM_INT({
    var jsString = document.location.href;
    var lengthBytes = lengthBytesUTF8(jsString)+1;
    var stringOnWasmHeap = _malloc(lengthBytes);
    stringToUTF8(jsString, stringOnWasmHeap, lengthBytes+1);
    return stringOnWasmHeap;
    });

  which = 'z';
  auto pos = href.find("?");

  if(pos == string::npos) {
    param = "";
    mainpart = href;
    }
  else {
    mainpart = href.substr(0, pos);
    which = href[pos+1];
    auto p = decodeURI(href.c_str()+pos+3);
    param = p;
    free((void*) p);
    }

  if(which == 'z') do_main_page();
  else if(which == 'g') do_explain_game(param);
  else if(which == 'b') do_explain_by(param);
  else if(which == 't') do_explain_tag(param);
  else if(which == 'x') do_explain_group(param);
  else do_main_page();

  return 0;
  }

void validate(game *g) {
  if(!g) return;
  auto add_tags = [g] (string s) {
    while(true) {
      auto pos = s.find("{");
      if(pos == string::npos) break;
      s = s.substr(pos + 1);
      int lpos = s.find("}");
      if(lpos == string::npos) break;
      string link = s.substr(0, lpos);
      s = s.substr(lpos + 1);
      if(link.substr(0, 4) == "tag:") { }
      else if(link.substr(0, 6) == "group:") { }
      else if(link.substr(0, 3) == "by:") { }
      else g->links_to.insert(link);
      }
    };
  for(auto s: g->reviews) add_tags(s);
  for(auto [t, s]: g->tagcomments) add_tags(s);
  }

extern "C" {
  void start(bool mobile) { init(mobile); }

  void explain_group(const char *s) { do_explain_group(s); }
  void explain_tag(const char *s) { do_explain_tag(s); }
  void explain_game(const char *s) { do_explain_game(s); }
  void explain_by(const char *s) { do_explain_by(s); }

  void on_find_change(const char *filter, const char *sorting, const char *color) {
    game_search(filter, sorting, color);
    stringstream ss;
    ss << "<br/>" << find_log.str();
    ss << "<br/>Number of games: " << found.size();
    if(found.size()) {
      ss << "<ul>";
      map<ld, vector<game*>> games_graph;
      for(auto g: found) games_graph[g->current_sort_value].emplace_back(g);
      int maxcat = 0;
      for(auto& [val, lst]: games_graph) {
        ss << "<li><b>" << val << ":</b>";
        int id = 0;
        maxcat = max<int>(maxcat, lst.size());
        for(auto g: lst) {
          char col[20];
          sprintf(col, "%06X", g->current_color_value);
          if(id++) ss << ",";
          ss << " <a onClick=\"explain_game('" << g->title << "')\"><font color=\"" << col << "\">" << g->title << "</font></a>";
          }
        ss << "</li>";
        }
      ss << "</ul>";

      ss << "<center><table cellpadding=0 cellspacing=0>\n";
      for(auto& [val, lst]: games_graph) {
        ss << "<tr><td style=\"celling-top:0px;\">" << val << "&nbsp;</td>";
        int sofar = 0;
        double width = 1000, height = 16;
        ss << "<td style=\"celling-top:0px;\"><svg style=\"border: 0px;\" width=\"" << width << "\" height=\"" << height << "\">";

        map<unsigned, int> qty;
        for(auto& g: lst) {
          qty[-g->current_color_value]++;
          }

        for(auto p: qty) {
          double left = sofar * width / maxcat;
          sofar += p.second;
          double right = sofar * width / maxcat;
          ss << "<path d=\"M " << left << " 0 L " << left << " " << height << " L " << right << " " << height << " L " << right << " 0 Z\" ";
          char col[20];
          sprintf(col, "%06X", -p.first);
          ss << "style=\"stroke:#000000;stroke-opacity:1;stroke-width:1px;fill:#" << col << ";fill-opacity:1\"/>";
          }
        ss << "</svg></td></tr>\n";
        }

      ss << "</table></center>";
      ss << "<br/>";
      }
    ss << "<br/>";

    set_value("gamelist", ss.str());
    }

  void find_games() {
    stringstream ss;
    for(auto g: found) {
      ss << "<hr/>";
      display(ss, g);
      }
    ss << "<hr/>\n";
    add_buttons(ss);
    set_value("output", ss.str());
    }

  void main_page() { do_main_page(); }
  }
