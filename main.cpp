#undef _GLIBCXX_DEBUG                // disable run-time bound checking, etc
#pragma GCC optimize("Ofast,inline") // Ofast = O3,fast-math,allow-store-data-races,no-protect-parens

#pragma GCC target("bmi,bmi2,lzcnt,popcnt")                      // bit manipulation
#pragma GCC target("movbe")                                      // byte swap
#pragma GCC target("aes,pclmul,rdrnd")                           // encryption
#pragma GCC target("avx,avx2,f16c,fma,sse3,ssse3,sse4.1,sse4.2") // SIMD

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <array>
#include <queue>
#include <unordered_set>
#include <unordered_map>

constexpr int NEIGH_SIZE = 6;
constexpr int PLAYER_SIZE = 2;
constexpr int INF = 1000000000;
constexpr int INVALID_ID = -1;

constexpr int CRYSTAL_PER_ANT_TO_MAXIMIZE_EGGS = 10;
constexpr double EGGS_KOEF_LV_MAX = 10.;
constexpr double EGGS_KOEF_LV_MAX_BUT_NO_CRYSTALS = 2.;
constexpr double EGGS_KOEF_LV_NORMAL = 3.;
constexpr double EGGS_KOEF_LV_NORMAL_BUT_NO_CRYSTALS = .5;

class player_t;

class cell_t {
public:
  void init (const int id, std::vector<cell_t> &cells);
  void read ();
  void set_resources_value (const double eggs_koef, const double crystals_coef);
  inline int id () const { return m_id; }
  inline int beacon () const { return m_beacon; }
  inline int opp_chain_val () const { return m_opp_chain_val; }
  inline const std::vector<cell_t*> &neighs () const { return m_neigh_cells; }
  inline bool is_egg () const { return m_type == 1 && m_resources > 0; }
  inline int resources () const { return m_resources; }
  inline int resources_value () const { return m_resources_value; }
  inline int ants (const int player_id) { return m_ants[player_id]; }
  inline void add_beacon (const int val = 1) { m_beacon += val; }
  inline int set_min_beacon (const int min_val) { const int add_val = std::max (min_val - m_beacon, 0); m_beacon += add_val; return add_val; }
  inline void set_chain_parent (const cell_t * const parent) { if (!parent) m_chain_len = 0; else { m_chain_len = parent->m_chain_len + 1; m_chain_parent = parent; }}
  inline void set_opp_chain_val (const int opp_chain_val) { m_opp_chain_val = opp_chain_val; }
private:
  int m_id = INVALID_ID;
  int m_type = -1;
  int m_resources = -1;
  int m_resources_value = -1;
  int m_beacon = -1;
  std::array<int, NEIGH_SIZE> m_neigh;
  std::array<int, PLAYER_SIZE> m_ants;

  std::vector<cell_t*> m_neigh_cells;

  int m_chain_len = -1; // for computing on step
  const cell_t *m_chain_parent = nullptr; // for computing on step

  int m_opp_chain_val = -1; // enemy
};
class map_t {
public:
  map_t (const std::vector<cell_t> &cells);
  inline int operator() (const int from, const int to) const { return m_dist[from][to]; }
private:
  std::vector<std::vector<int>> m_dist;
  const std::vector<cell_t> &m_cells;
};
class player_t {
public:
  void init (const int id, const int number_of_bases, std::vector<cell_t> &cells);
  void read_score () { std::cin >> m_score; std::cin.ignore (); };
  void update_step (std::vector<cell_t> &cells);
  inline const std::vector<cell_t*> &bases () const { return m_bases; }
  inline int ants_cnt () const { return m_ants_cnt; }
  inline int ants_cnt_free () const { return m_ants_cnt_free; }
  inline void use_ants (const int cnt) { m_ants_cnt_free -= cnt; }
  inline int id () const { return m_id; }
  inline int score () const { return m_score; }
private:
  int m_id = INVALID_ID;
  std::vector<cell_t*> m_bases;
  int m_score = -1;

  int m_ants_cnt;
  int m_ants_cnt_free;
  std::vector<cell_t*> m_ants; // size != ants count
};
class game_t {
public:
  game_t ();
  void read_step ();
  void play_step ();
private:
  std::vector<cell_t> m_cells;
  std::array<player_t, PLAYER_SIZE> m_players;
  std::unique_ptr<map_t> m_map;
  std::vector<cell_t *> m_aims;
  int m_crystals = -1;
  int m_eggs = -1;

  void compute_aims ();
  void compute_enemy_chain ();
  void fill_beacons ();
  void set_min_beacon (cell_t &cell, const int min_beacon);

  inline int dist (const int from, const int to) { return (*m_map) (from, to); }

  std::string m_actions_text;
  void commit_beacon (const int cell_id, const int strength) { m_actions_text += "BEACON " + std::to_string (cell_id) + " " + std::to_string (strength) + ";"; }
  void commit_line (const int cell_id1, const int cell_id2, const int strength) { m_actions_text += "LINE " + std::to_string (cell_id1) + " " + std::to_string (cell_id2) + " " + std::to_string (strength) + ";"; }
  void commit_wait () { m_actions_text += "WAIT;"; }
  void commit_message (const std::string &msg) { m_actions_text += msg + ";"; }

  clock_t m_start_step_time;
  void start_step_timer () { m_start_step_time = clock (); }
  void stop_step_timer ();
};

void cell_t::init (const int id, std::vector<cell_t> &cells) {
  m_id = id;
  m_neigh_cells.reserve (NEIGH_SIZE);
  std::cin >> m_type >> m_resources; std::cin.ignore ();
  for (int &neigh_id : m_neigh) {
    std::cin >> neigh_id, std::cin.ignore ();
    if (neigh_id != INVALID_ID)
      m_neigh_cells.push_back (&cells[neigh_id]);
  }
}
void cell_t::read () {
  std::cin >> m_resources >> m_ants[0] >> m_ants[1]; std::cin.ignore ();
  m_resources_value = 0;
  m_beacon = 0;
  m_opp_chain_val = 0;
  m_chain_len = -1;
  m_chain_parent = nullptr;
}

void cell_t::set_resources_value (const double eggs_koef, const double crystals_coef) {
  const double koef = is_egg () ? eggs_koef : crystals_coef;
  m_resources_value = m_resources * koef;
}

map_t::map_t (const std::vector<cell_t> &cells)
 : m_dist (std::vector<std::vector<int>> (cells.size (), std::vector<int> (cells.size (), INF)))
 , m_cells (cells) {
  struct temp_data_t {
    const cell_t &cell;
    const int dist;
  };
  for (const cell_t &cell_from : m_cells) {
    std::queue<temp_data_t> q;
    q.push ({cell_from, 0});
    while (!q.empty ()) {
      const cell_t &cell_to = q.front ().cell;
      const int dist = q.front ().dist;
      q.pop ();
      if (m_dist[cell_from.id ()][cell_to.id ()] <= dist)
        continue;
      m_dist[cell_from.id ()][cell_to.id ()] = dist;
      for (const cell_t * const next_cell : cell_to.neighs ())
        if (m_dist[cell_from.id ()][next_cell->id ()] > dist + 1)
          q.push ({*next_cell, dist + 1});
    }
  }
}

void player_t::init (const int id, const int number_of_bases, std::vector<cell_t> &cells) {
  m_id = id;
  m_bases.reserve (number_of_bases);
  for (int i_base = 0; i_base < number_of_bases; i_base++) {
    int i_cell;
    std::cin >> i_cell; std::cin.ignore ();
    m_bases.push_back (&cells[i_cell]);
  }
}

void player_t::update_step (std::vector<cell_t> &cells) {
  m_ants_cnt = 0;
  m_ants.clear ();
  for (cell_t &cell : cells)
    if (cell.ants (m_id) > 0) {
      m_ants_cnt += cell.ants (m_id);
      m_ants.push_back (&cell);
    }
  m_ants_cnt_free = m_ants_cnt;
}

game_t::game_t () {
  int number_of_cells;
  std::cin >> number_of_cells; std::cin.ignore ();
  m_cells.resize (number_of_cells);
  for (int i_cell = 0; i_cell < number_of_cells; ++i_cell)
    m_cells[i_cell].init (i_cell, m_cells);
  m_aims.reserve (number_of_cells);
  int number_of_bases;
  std::cin >> number_of_bases; std::cin.ignore ();
  for (int i_player = 0; i_player < PLAYER_SIZE; i_player++)
    m_players[i_player].init (i_player, number_of_bases, m_cells);
  m_map = std::make_unique<map_t> (m_cells);
}
void game_t::read_step () {
  start_step_timer ();
  m_actions_text = "";
  m_aims.clear ();
  for (player_t &player : m_players)
    player.read_score ();
  for (cell_t &cell : m_cells)
    cell.read ();
  for (player_t &player : m_players)
    player.update_step (m_cells);
}
void game_t::play_step () {
  compute_aims ();
  compute_enemy_chain ();
  fill_beacons ();
  commit_wait ();
  stop_step_timer ();
  std::cout << m_actions_text << std::endl;
}
void game_t::compute_aims () {
  const player_t &iam = m_players[0];
  const player_t &enemy = m_players[1];

  //int nearest_crystal
  m_crystals = 0;
  m_eggs = 0;
  for (cell_t &cell : m_cells) {
    if (cell.resources () > 0) {
      if (cell.is_egg ()) m_eggs += cell.resources ();
      else                m_crystals += cell.resources ();
    }
  }

  //const int score_to_win = (iam.score () + enemy.score () + m_crystals) / 2;
  //if (iam.score () + iam.ants_cnt () * )

  for (cell_t &cell : m_cells) {
    if (cell.resources () > 0) {
      m_aims.push_back (&cell);
    }
  }

  double eggs_koef = 1. * iam.bases ().size ();
  if (iam.ants_cnt () <= enemy.ants_cnt ()) {
    if (m_crystals > CRYSTAL_PER_ANT_TO_MAXIMIZE_EGGS * iam.ants_cnt ())
      eggs_koef *= EGGS_KOEF_LV_MAX;
    else
      eggs_koef *= EGGS_KOEF_LV_MAX_BUT_NO_CRYSTALS;
  }
  else {
    if (m_crystals > CRYSTAL_PER_ANT_TO_MAXIMIZE_EGGS * iam.ants_cnt ())
      eggs_koef *= EGGS_KOEF_LV_NORMAL;
    else
      eggs_koef *= EGGS_KOEF_LV_NORMAL_BUT_NO_CRYSTALS;
  }

  double crystals_koef = 1.;

  for (cell_t * const aim_cell : m_aims)
    aim_cell->set_resources_value (eggs_koef, crystals_koef);
}
void game_t::compute_enemy_chain () {
  const player_t &enemy = m_players[1];
  struct temp_data_t {
    const cell_t *parent;
    int min_ants_cnt;
  };
  std::unordered_map<cell_t *, temp_data_t> path; // cell -> parent
  std::queue<temp_data_t> q;
  for (cell_t * const base_cell : enemy.bases ()) {
    q.push ({base_cell, base_cell->ants (enemy.id ())});
    path.insert ({base_cell, temp_data_t {nullptr, base_cell->ants (enemy.id ())}});
  }

  std::unordered_set<cell_t *> edges;
  while (!q.empty ()) {
    const cell_t * const parent = q.front ().parent;
    const int min_ants_cnt = q.front ().min_ants_cnt;
    q.pop ();
    for (cell_t * const next_cell : parent->neighs ()) {
      if (next_cell->ants (enemy.id ()) <= 0)
        continue;
      const int new_min_ants_cnt = std::min (next_cell->ants (enemy.id ()), min_ants_cnt);
      if (path.count (next_cell)) {
        temp_data_t &temp_data = path[next_cell];
        if (temp_data.min_ants_cnt < new_min_ants_cnt) {
          temp_data.min_ants_cnt = new_min_ants_cnt;
          q.push ({next_cell, new_min_ants_cnt});
        }
      }
      else {
        path.insert ({next_cell, temp_data_t {parent, new_min_ants_cnt}});
        q.push ({next_cell, new_min_ants_cnt});
      }
    }
  }
  for (auto &path_el : path) {
    cell_t * const cell = path_el.first;
    const int min_ants_cnt = path_el.second.min_ants_cnt;
    cell->set_opp_chain_val (min_ants_cnt);
  }
  //for (const cell_t &cell : m_cells)
  //  if (cell.opp_chain_val () > 0)
  //    std::cerr << "# " << cell.id () << " " << cell.opp_chain_val () << std::endl;
}
void game_t::fill_beacons () {
  player_t &iam = m_players[0];
  std::unordered_map<cell_t*, int> path;
  //std::vector<std::unordered_map<cell_t*, int>> lines;
  //std::unordered_map<cell_t*, int> *new_line;

  for (cell_t * const base_cell : iam.bases ()) {
    path.insert ({base_cell, 1});
    iam.use_ants (1);
    base_cell->set_chain_parent (nullptr);
    //base_cell->set_min_beacon (1);
  }

  std::unordered_set<cell_t *> aims (m_aims.begin (), m_aims.end ());
  struct temp_data_t {
    int min_dis, val_cnt;
    const cell_t * const parent;
  };
  std::unordered_map<cell_t *, temp_data_t> candidates;

  while (!aims.empty () && iam.ants_cnt_free () > 0) {
    candidates.clear ();
    for (const auto &path_el : path) {
      const cell_t * const cell_from = path_el.first;
      //int enemy_line_power =
      for (cell_t * const next_cell : cell_from->neighs ())
        if (!path.count (&*next_cell)) {
          const auto it_ins = candidates.insert ({&*next_cell, temp_data_t {INF, 0, cell_from}});
          if (!it_ins.second)
            continue;
          const cell_t * const new_cell = it_ins.first->first;
          temp_data_t &new_cell_data = it_ins.first->second;
          for (const cell_t * const aim_cell : aims) {
            const int dist_to_aim = dist (new_cell->id (), aim_cell->id ());
            if (new_cell_data.min_dis > dist_to_aim) {
              new_cell_data.min_dis = dist_to_aim;
              new_cell_data.val_cnt = aim_cell->resources_value ();
            }
            else if (new_cell_data.min_dis == dist_to_aim)
              new_cell_data.val_cnt += aim_cell->resources_value ();
          }
        }
    }
    if (candidates.empty ())
      break;
    const auto &best_cond_it = std::min_element (candidates.begin (), candidates.end (),
      [this, &iam] (const auto &a, const auto &b) {
        if (a.second.min_dis != b.second.min_dis)
          return a.second.min_dis < b.second.min_dis;
        if (a.second.val_cnt != b.second.val_cnt)
          return a.second.val_cnt > b.second.val_cnt;
        return dist (a.first->id (), iam.bases ().front ()->id ()) < dist (b.first->id (), iam.bases ().front ()->id ());
      });
    cell_t * const add_cell = best_cond_it->first;
    const cell_t * const parent_cell = best_cond_it->second.parent;
    add_cell->set_chain_parent (parent_cell);
    //add_cell->set_min_beacon (parent_cell->beacon ());
    path.insert ({add_cell, 1});
    iam.use_ants (1);
    if (aims.erase (add_cell) > 0) { // we found aim
      //const int max_cnt = add_cell->resources ();

      // TO-DO: count ants
    }
    //std::cerr << "# " << add_cell->id () << " " << best_cond_it->second.min_dis << " " << best_cond_it->second.val_cnt << std::endl;
  }

  for (const auto &path_el : path) {
    cell_t * const cell = path_el.first;
    const int beacon = path_el.second;
    cell->set_min_beacon (beacon);
  }

  for (const cell_t &cell : m_cells)
    if (cell.beacon () > 0)
      commit_beacon (cell.id (), cell.beacon ());
}

void game_t::set_min_beacon (cell_t &cell, const int min_beacon) {
  const int need_use_ants = cell.set_min_beacon (min_beacon);
  m_players[0].use_ants (need_use_ants);
}

void game_t::stop_step_timer () {
  static double max_loop_time = 0., sum_time = 0.;
  const double spend_time = static_cast<double> (clock () - m_start_step_time) / CLOCKS_PER_SEC;
  max_loop_time = std::max (max_loop_time, spend_time);
  sum_time += spend_time;
  commit_message ("MESSAGE " + std::to_string (int(spend_time * 1000000)) + "/" + std::to_string (int(max_loop_time * 1000000)));
  std::cerr << spend_time * 1000 << "ms / " << max_loop_time * 1000 << "ms / " << sum_time * 1000 << "ms" << std::endl;
}

int main()
{
  game_t game;
  while (1) {
    game.read_step ();
    game.play_step ();
  }
}
