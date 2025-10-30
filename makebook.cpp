/* how to run:
 mkdir printout
 g++ makebook.cpp -o makebook
 ./makebook > printout/printout.tex
 cd printout
 makeindex printout
 pdflatex printout
 evince printout.pdf
 cd ..
 (as usual with Latex, might be useful to run multiple times to get all the references right */
 
#include <string>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <algorithm>

using namespace std;

void plog(string s) {
  printf("%s\n", s.c_str());
  }

string its(int x) { return to_string(x); }

#include "readdata.cpp"

void out_tag_value(game *g, const string &tag) {
  fflush(stdout);
  if(!is_numeric[tag] && (g->values[tag] == 0 || g->values[tag] == 1))
     printf("%s", g->values[tag] ? "{\\checkmark} " : "{$\\times$} ");
  else if(g->values[tag] == (int) g->values[tag]) printf("=%d", int(g->values[tag]));
  else printf("=%f", g->values[tag]);
  if(g->qtags.count(tag)) printf("%s", "$^?$");
  }

void out_desc(string s) {
  bool in_link = false;
  string link = "";
  string out = s;
  auto rep = [&] (string s1, string s2) {
    while(true) {
      int pos = out.find(s1);
      if(pos == string::npos) break;
      out.replace(pos, s1.size(), s2);
      }
    };
  rep("%", "PERCENT");
  rep("PERCENT", "\\%");
  rep("<br/><br/>", "\n\n");
  rep("&", "AND");
  rep("AND", "\\&");
  rep("$", "DOLLAR");
  rep("DOLLAR", "\\$");
  rep("<b>", "OBRACE\\bf ");
  rep("</b>", "CBRACE");
  rep("<i>", "OBRACE\\it ");
  rep("</i>", "CBRACE");
  if(auto p1 = out.find("<a href=\""); p1 != string::npos) {
    auto p2 = out.find("\">");
    auto p3 = out.find("</a>");
    string link = out.substr(p1 + 9, p2 - p1 - 9);
    out.replace(p3, 4, "\\footnoteOBRACE\\urlOBRACE" + link + "CBRACECBRACE");
    out.replace(p1, p2-p1+2, "");
    }

  s = out; out = "";
  for(char c: s) {
    if(!in_link) {
      if(c != '{') out += c;
      else in_link = true, link = "";
      }
    else if(in_link) {
      if(c != '}') link += c;
      else {
        if(link.substr(0, 4) == "tag:") {
          auto tg = link.substr(4);
          out += "\\hyperlink{tag:" + tg + "}{\\sc " + tg + "}";
          }
        else if(link.substr(0, 6) == "group:") {
          auto gr = link.substr(6);
          out += "{\\sc " + gr + "}";
          }
        else if(link.substr(0, 3) == "by:") {
          auto by = link.substr(3);
          out += "{\\sc " + by + "}";
          }
        else {
          out += "\\hyperlink{" + link + "}{\\sc " + link + "}";
          }
        in_link = false;
        }
      }
    }
  rep("OBRACE", "{");
  rep("CBRACE", "}");
  rep("->", "$\\rightarrow$");
  printf("%s\n", out.c_str());
  }

void out_game(game *g) {
  printf("\\subsection{%s}\\label{%s}\n", g->title.c_str(), g->title.c_str());

  for(auto &png: g->pngs) {
    printf("\\begin{figure}\\begin{center}\\includegraphics[width=0.8\\textwidth]{../pngs/%s.png}\\end{center} \\caption{", png.first.c_str());
    out_desc(png.second);
    printf("}\\end{figure}\n");
    }

  printf("\\index{%s}\n", g->title.c_str());
  for(auto s: g->other_titles) 
    printf("\\index{%s}\n", s.c_str());
  printf("\\hypertarget{%s}{}\n", g->title.c_str());
  for(auto s: g->other_titles) 
    printf("\\hypertarget{%s}{}\n", s.c_str());
  printf("Tags:");

  for(auto gr: all_groups) {
    for(auto t: tags_in_group[gr]) if(g->values.count(t)) {
      printf(" \\hyperlink{tag:%s}{\\sc %s}", t.c_str(), t.c_str());
      out_tag_value(g, t);
      }
    }
  printf("\n\n");
  for(auto& b: g->by) printf("By \\hyperlink{by:%s}{%s}\n\n", b.c_str(), b.c_str());

  for(auto &ff: g->reviews) {
    out_desc(ff);
    printf("\n\n");
    }

  if(g->spoilers.size()) printf("(Spoilers not included in the book.)");

  for(auto [t, s]: g->tagcomments) {
    printf("Tag comment for {\\sc %s}: ", t.c_str());
    out_desc(s);
    printf("\n\n");
    }

  }

void out_dev(string dev) {
  printf("\\paragraph{{\\sc %s}}\\label{by:%s}\\hypertarget{by:%s}{}\n", dev.c_str(), dev.c_str(), dev.c_str());
  printf("\\index{%s}\n", dev.c_str());
  if(about_dev.count(dev)) out_desc(about_dev[dev]);
  else printf("\\rule{0cm}{0cm}");
  printf("\n\n");
  printf("Games:"); 
  int id = 0;
  for(auto& g: games) {
    if(is_by(g, dev)) {
      if(id > 0) printf(","); id++;
      printf(" \\hyperlink{%s}{{\\sc %s}}", g->title.c_str(), g->title.c_str());
      }
    }
  printf("\n\n");
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

int main() {
  parse("zgdb.txt");
  printf("\\documentclass{article}\n");
  printf("\\usepackage{graphicx}\n");
  printf("\\usepackage{url}\n");
  printf("\\usepackage{amssymb}\n");
  printf("\\usepackage{hyperref}\n");
  printf("\\usepackage{makeidx}\n");
  printf("\\makeindex\n");
  printf("\\begin{document}\n");
  printf("This is ZGDb in the book form.\n");

  printf("\\section{Tags}\n");
  for(auto gr: all_groups) {
    printf("\\subsection{%s}\n", gr.c_str());
    out_desc(groupdesc[gr]);
    for(auto tag: tags_in_group[gr]) {
      printf("\\paragraph{{\\sc %s}}\n", tag.c_str());
      printf("\\index{%s}\n", tag.c_str());
      printf("\\hypertarget{tag:%s}{}\n", tag.c_str());
      printf("\\label{tag:%s}\n", tag.c_str());
      out_desc(tagdesc[tag]);
      }
    }

  vector<game*> all_games = games;
  auto gplay = [&] (game *g) { if(g->values.count("played")) return g->values["played"]; else return 0.; };
  sort(all_games.begin(), all_games.end(), [&] (game *g1, game *g2) { return make_pair(gplay(g1), g1->title) < make_pair(gplay(g2), g2->title); });
  printf("\\section{Philosophies}\n");
  for(auto g: all_games) if(g->values.count("philosophy")) out_game(g);

  printf("\\section{Games played}\n");
  for(auto g: all_games) if(!g->values.count("philosophy")) if(gplay(g)) out_game(g);

  printf("\\section{Games not played}\n");
  for(auto g: all_games) if(!g->values.count("philosophy")) if(!gplay(g)) out_game(g);

  printf("\\section{Developers}\n");
  for(auto [d, q]: all_devs) if(d.substr(0, 7) != "country") out_dev(d);

  printf("\\section{Countries}\n");
  for(auto [d, q]: all_devs) if(d.substr(0, 7) == "country") out_dev(d);

  printf("\\printindex\n");
  printf("\\end{document}\n");
  return 0;
  }
