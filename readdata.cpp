const double TRUTH = 1;
using ld = double;

struct game {
  string title;
  string file;
  map<string, double> values;
  vector<pair<string, string>> pngs;
  vector<string> reviews;
  vector<string> by;
  set<string> qtags;
  ld current_sort_value;
  unsigned current_color_value;
  unsigned current_random_value;
  map<string, string> tagcomments;
  set<string> links_to;
  set<string> linked_by;
  vector<string> other_titles;
  };

string about;

map<string, string> tagdesc;
map<string, string> groupdesc;
map<string, string> which_group;

map<string, vector<string>> tags_in_group;

vector<string> all_groups;

map<string, bool> is_numeric;

vector<game*> games;

string junk;
string* addto = &junk;
bool auto_break = false;

map<string, game*> game_by_name;

vector<string> validators;
vector<pair<string, string>> suggestors;

map<string, int> all_devs;

map<string, string> about_dev;

void validate(game *g);

void parse(const string& fname) {
  string current_group;
  ifstream f(fname);
  string s;
  bool last_empty;

  game *g = nullptr;
  auto where = [&] { return "{" + g->file + ":" + g->title + "}"; };

  while(getline(f, s)) {
    if(s == "") {last_empty = true; continue; }

    string cap, param;
    int pos = s.find(" ");
    if(pos != string::npos)  { cap = s.substr(0, pos); param = s.substr(pos+1); }
    else { cap = s; param = ""; }

    if(cap == "INCLUDE") {
      parse(param);
      }
    
    else if(cap == "GROUP") {
      addto = &(groupdesc[current_group = param]); last_empty = false;
      all_groups.push_back(current_group);
      }

    else if(cap == "TAG" || cap == "NUMTAG" ) {
      if(cap == "NUMTAG") is_numeric[param] = true;
      tags_in_group[current_group].push_back(param);
      which_group[param] = current_group;
      addto = &(tagdesc[param]); last_empty = false;
      }

    else if(cap == "ABOUTBY" ) {
      addto = &(about_dev[param]); last_empty = false;
      }

    else if(cap == "GAME") {
      validate(g);
      g = new game;
      g->title = param;
      g->file = fname;
      // plog("reading " + param);
      games.push_back(g);
      if(game_by_name.count(param)) plog("REPEATED GAME " + param);
      game_by_name[param] = g;

      g->reviews.emplace_back();
      addto = &g->reviews.back();
      last_empty = false; continue;
      }

    else if(cap == "AKA") {
      if(game_by_name.count(param)) plog("REPEATED GAME " + param);
      game_by_name[param] = g;
      g->other_titles.push_back(param);
      }

    else if(cap == "REV") {
      plog("extra review");
      g->reviews.emplace_back();
      addto = &g->reviews.back();
      last_empty = false; continue;
      }

    else if(cap == "ABOUT") {
      addto = &about;
      last_empty = false; continue;
      }

    else if(cap == "VALIDATE") {
      validators.push_back(param);
      if(*addto != "") addto = &junk;
      }

    else if(cap == "SUGGEST") {
      auto pos = param.find(" ");
      string stag = param.substr(0, pos);
      string srule = param.substr(pos+1);
      suggestors.emplace_back(stag, srule);
      }

    else if(cap == "VAL") {
      string c = param + " ";
      string cur = "";
      for(char ch: c) {
        if(ch == ' ') {
          auto pos = cur.find("=");
          if(pos == string::npos) {
            g->values[cur] = TRUTH;
            if(which_group[cur] == "") plog(where() + " unknown tag: " + cur);
            }
          else {
            string tag = cur.substr(0, pos);
            string subparam = cur.substr(pos+1);
            if(subparam == "?") {
              if(!is_numeric[tag]) g->values[tag] = 0.5;
              g->qtags.insert(tag);
              }
            else {
              double d; sscanf(subparam.c_str(), "%lf", &d);
              g->values[tag] = d;
              if(which_group[tag] == "") plog(where() + " unknown tag/value: " + cur);
              }
            }
          cur = "";
          }
        else cur += ch;
        }

      if(*addto != "") addto = &junk;
      }

    else if(cap == "#") {}

    else if(cap == "BY") {
      g->by.push_back(param);
      all_devs[param]++;
      if(*addto != "") addto = &junk;
      }

    else if(cap == "TAGCMT") {
      if(which_group[param] == "") plog(where() + " unknown tag/value for tagcomment: " + param);
      addto = &(g->tagcomments[param]);
      }

    else if(cap == "PNG") {
      pos = param.find(" ");
      if(pos == string::npos) { plog(where() + " bad png format " + param); continue; }
      g->pngs.emplace_back(param.substr(0, pos), param.substr(pos+1));
      if(*addto != "") addto = &junk;
      }

    else {
      if(last_empty && *addto != "") *addto += "<br/><br/>";
      *addto += s; *addto += " "; auto_break = true;
      }

    last_empty = false;
    }

  validate(g);
  if(junk != "") {
    printf("JUNK contains:\n");
    printf("%s\n", junk.c_str());
    junk = "";
    }
  }

