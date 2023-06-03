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
#include <set>

constexpr int NEIGH_SIZE = 6;
constexpr int PLAYER_SIZE = 2;
constexpr int INF = 1000000000;
constexpr int INVALID_ID = -1;

constexpr int CRYSTAL_PER_ANT_TO_MAXIMIZE_EGGS = 10;
constexpr double EGGS_KOEF_LV_MAX = 10.;
constexpr double EGGS_KOEF_LV_MAX_BUT_NO_CRYSTALS = 2.;
constexpr double EGGS_KOEF_LV_NORMAL = 3.;
constexpr double EGGS_KOEF_LV_NORMAL_BUT_NO_CRYSTALS = .5;
constexpr double ANTS_STOP_COLLECT_EGGS = 1;

class player_t;

class cell_t {
public:
  void init (const int id, std::vector<cell_t> &cells);
  void read ();
  void set_resources_value (const double eggs_koef, const double crystals_coef);
  inline int id () const { return m_id; }
  inline int beacon () const { return m_beacon; }
  inline int ants_chain_power (const int player_id) const { return m_ants_chain_power[player_id]; }
  inline const std::vector<cell_t*> &neighs () const { return m_neigh_cells; }
  inline bool is_egg () const { return m_type == 1 && m_resources > 0; }
  inline int resources () const { return m_resources; }
  inline int resources_value () const { return m_resources_value; }
  inline int ants (const int player_id) { return m_ants_cnt[player_id]; }
  inline void add_beacon (const int val = 1) { m_beacon += val; }
  inline int set_min_beacon (const int beacon);
  inline void set_chain_parent (const cell_t * const parent) { if (!parent) m_chain_len = 0; else { m_chain_len = parent->m_chain_len + 1; m_chain_parent = parent; }}
  inline void set_ants_chain_power (const int player_id, const int ants_chain_power) { m_ants_chain_power[player_id] = ants_chain_power; }
private:
  int m_id = INVALID_ID;
  int m_type = -1;
  int m_resources = -1;
  int m_resources_value = -1;
  int m_beacon = -1;

  std::array<int, NEIGH_SIZE> m_neigh_cell_ids;
  std::vector<cell_t*> m_neigh_cells;

  std::array<int, PLAYER_SIZE> m_ants_cnt;
  std::array<int, PLAYER_SIZE> m_ants_chain_power;

  int m_chain_len = -1; // for computing on step
  const cell_t *m_chain_parent = nullptr; // for computing on step
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
  void init (const int id, const int number_of_bases, std::vector<cell_t> &cells, player_t * const enemy);
  void read_score () { std::cin >> m_score; std::cin.ignore (); };
  void update_step (std::vector<cell_t> &cells);
  inline const std::vector<cell_t*> &bases () const { return m_base_cells; }
  inline const std::set<cell_t*> &bases_set () const { return m_base_set_cells; }
  inline int ants_cnt () const { return m_ants_cnt; }
  inline int ants_cnt_free () const { return m_ants_cnt_free; }
  inline int ants_cnt_add_next_step () const { return m_ants_cnt_add_next_step; }
  inline void use_ants (const int cnt) { m_ants_cnt_free -= cnt; }
  inline int id () const { return m_id; }
  inline int score () const { return m_score; }
private:
  int m_id = INVALID_ID;
  std::vector<cell_t*> m_base_cells;
  std::set<cell_t*> m_base_set_cells;
  player_t *m_enemy;
  int m_score = -1;

  int m_ants_cnt;
  int m_ants_cnt_free;
  std::vector<cell_t*> m_ants_cells; // size != ants count

  int m_ants_cnt_add_next_step = 0;
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
  std::vector<cell_t *> m_all_aim_cells;
  std::vector<cell_t *> m_egg_cells;
  int m_crystals_cnt = -1;
  int m_eggs_cnt = -1;

  bool m_inital_read_flag = true;
  std::vector<cell_t *> m_initial_aims; // for each base

  void compute_aims ();
  void compute_ants_chain_power ();
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
  for (int &neigh_id : m_neigh_cell_ids) {
    std::cin >> neigh_id, std::cin.ignore ();
    if (neigh_id != INVALID_ID)
      m_neigh_cells.push_back (&cells[neigh_id]);
  }
}
void cell_t::read () {
  std::cin >> m_resources >> m_ants_cnt[0] >> m_ants_cnt[1]; std::cin.ignore ();
  m_resources_value = 0;
  m_beacon = 0;
  for (int &ants_chain_power : m_ants_chain_power)
    ants_chain_power = 0;
  m_chain_len = -1;
  m_chain_parent = nullptr;
}

void cell_t::set_resources_value (const double eggs_koef, const double crystals_coef) {
  const double koef = is_egg () ? eggs_koef : crystals_coef;
  m_resources_value = m_resources * koef;
}

int cell_t::set_min_beacon (const int beacon) {
  const int add_val = std::max (beacon - m_beacon, 0);
  m_beacon += add_val;
  return add_val;
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

void player_t::init (const int id, const int number_of_bases, std::vector<cell_t> &cells, player_t * const enemy) {
  m_id = id;
  m_base_cells.reserve (number_of_bases);
  for (int i_base = 0; i_base < number_of_bases; i_base++) {
    int i_cell;
    std::cin >> i_cell; std::cin.ignore ();
    m_base_cells.push_back (&cells[i_cell]);
    m_base_set_cells.insert (&cells[i_cell]);
  }
  m_enemy = enemy;
}

void player_t::update_step (std::vector<cell_t> &cells) {
  m_ants_cnt = 0;
  m_ants_cnt_add_next_step = 0;
  m_ants_cells.clear ();
  for (cell_t &cell : cells)
    if (cell.ants (m_id) > 0) {
      m_ants_cnt += cell.ants (m_id);
      m_ants_cells.push_back (&cell);
      if (cell.is_egg () && cell.ants_chain_power (m_id) > 0)
        m_ants_cnt_add_next_step += std::min (cell.resources (), cell.ants_chain_power (m_id));
    }
  m_ants_cnt_free = m_ants_cnt;
  m_ants_cnt_add_next_step *= m_base_cells.size ();
}

game_t::game_t () {
  int number_of_cells;
  std::cin >> number_of_cells; std::cin.ignore ();
  m_cells.resize (number_of_cells);
  for (int i_cell = 0; i_cell < number_of_cells; ++i_cell)
    m_cells[i_cell].init (i_cell, m_cells);
  m_all_aim_cells.reserve (number_of_cells);
  m_egg_cells.reserve (number_of_cells);
  int number_of_bases;
  std::cin >> number_of_bases; std::cin.ignore ();
  for (int i_player = 0; i_player < PLAYER_SIZE; i_player++)
    m_players[i_player].init (i_player, number_of_bases, m_cells, &m_players[(i_player + 1) % 2]);
  m_map = std::make_unique<map_t> (m_cells);
}
void game_t::read_step () {
  start_step_timer ();
  m_actions_text = "";
  m_all_aim_cells.clear ();
  m_egg_cells.clear ();
  m_crystals_cnt = 0;
  m_eggs_cnt = 0;
  for (player_t &player : m_players)
    player.read_score ();

  for (cell_t &cell : m_cells) {
    cell.read ();
    if (cell.is_egg ()) {
      m_egg_cells.push_back (&cell);
      m_eggs_cnt += cell.resources ();
    }
    else if (cell.resources () > 0) {
      m_crystals_cnt += cell.resources ();
    }
  }

  compute_ants_chain_power ();
  for (player_t &player : m_players)
    player.update_step (m_cells);

  if (m_inital_read_flag) {
    const cell_t *last_best_base = nullptr;
    const cell_t *last_best_egg = nullptr;
    std::vector<const cell_t *> my_bases_to_find_aim;
    for (const cell_t * const base_cell : m_players[0].bases ()) {
      bool aim_near_was = false;
      for (cell_t * const next_cell : base_cell->neighs ()) {
        if (next_cell->is_egg ()) {
          m_initial_aims.push_back (next_cell);
          aim_near_was = true;
        }
      }
      if (!aim_near_was)
        my_bases_to_find_aim.push_back (base_cell);
    }
    const size_t aim_near_cnt = m_initial_aims.size ();
    while (m_initial_aims.size () - aim_near_cnt < my_bases_to_find_aim.size () && m_initial_aims.size () < m_egg_cells.size ()) {
      const cell_t *best_base = nullptr;
      cell_t *best_egg = nullptr;
      int best_dist = INF;
      for (cell_t * const egg_cell : m_egg_cells) {
        if (last_best_egg == egg_cell) continue;
        for (const cell_t * const base_cell : my_bases_to_find_aim) {
          if (last_best_base == base_cell) continue;
          const int temp_dist = dist (base_cell->id (), egg_cell->id ());
          if (    temp_dist < best_dist
              || (temp_dist == best_dist && egg_cell->resources () > best_egg->resources ())) {
            best_dist = temp_dist;
            best_base = base_cell;
            best_egg = egg_cell;
          }
        }
      }
      if (best_dist > 4 || best_dist * 3 > m_players[0].ants_cnt ())
        break;
      last_best_base = best_base;
      last_best_egg = best_egg;
      m_initial_aims.push_back (best_egg);
    }
    if (m_initial_aims.size () == m_egg_cells.size () && m_egg_cells.size () == 2)
      m_initial_aims.pop_back ();
    m_inital_read_flag = false;
  }
}
void game_t::play_step () {
  compute_aims ();
  fill_beacons ();
  commit_wait ();
  stop_step_timer ();
  std::cout << m_actions_text << std::endl;
}
void game_t::compute_aims () {
  const player_t &iam = m_players[0];
  const player_t &enemy = m_players[1];

  if (!m_initial_aims.empty ()) {
    bool stop_initial_aims = false;
    for (const cell_t * const aim_cell : m_initial_aims)
      if (aim_cell->resources () <= 0) {
        stop_initial_aims = true;
        break;
      }

    if (stop_initial_aims || (m_crystals_cnt < m_eggs_cnt && iam.ants_cnt () > enemy.ants_cnt () * 1.2) || m_crystals_cnt < iam.ants_cnt ())
      m_initial_aims.clear ();
    else {
      for (cell_t * const aim_cell : m_initial_aims) {
        m_all_aim_cells.push_back (aim_cell);
        aim_cell->set_resources_value (1./*eggs_koef*/, 1./*crystals_koef*/);
      }
      return;
    }
  }

  const int score_to_win = (iam.score () + enemy.score () + m_crystals_cnt) / 2;
  const bool stop_collect_eggs = iam.score () + iam.ants_cnt () * ANTS_STOP_COLLECT_EGGS >= score_to_win;

  for (cell_t &cell : m_cells) {
    if (cell.resources () > 0) {
      if (stop_collect_eggs && cell.is_egg ())
        continue;
      m_all_aim_cells.push_back (&cell);
    }
  }

  double eggs_koef = 1. * iam.bases ().size ();
  //if (iam.ants_cnt () <= enemy.ants_cnt ()) {
  //  if (m_crystals > CRYSTAL_PER_ANT_TO_MAXIMIZE_EGGS * iam.ants_cnt ())
  //    eggs_koef *= EGGS_KOEF_LV_MAX;
  //  else
  //    eggs_koef *= EGGS_KOEF_LV_MAX_BUT_NO_CRYSTALS;
  //}
  //else {
  //  if (m_crystals > CRYSTAL_PER_ANT_TO_MAXIMIZE_EGGS * iam.ants_cnt ())
  //    eggs_koef *= EGGS_KOEF_LV_NORMAL;
  //  else
  //    eggs_koef *= EGGS_KOEF_LV_NORMAL_BUT_NO_CRYSTALS;
  //}
  eggs_koef = 1. * m_crystals_cnt / m_eggs_cnt * iam.bases ().size ();

  double crystals_koef = 1.;

  for (cell_t * const aim_cell : m_all_aim_cells)
    aim_cell->set_resources_value (eggs_koef, crystals_koef);
}
void game_t::compute_ants_chain_power () {
  struct temp_data_t {
    const cell_t *parent;
    int min_ants_cnt;
  };
  for (const player_t &player : m_players) {
    std::unordered_map<cell_t *, temp_data_t> path; // cell -> parent
    std::queue<temp_data_t> q;
    for (cell_t * const base_cell : player.bases ()) {
      q.push ({base_cell, base_cell->ants (player.id ())});
      path.insert ({base_cell, temp_data_t {nullptr, base_cell->ants (player.id ())}});
    }

    std::unordered_set<cell_t *> edges;
    while (!q.empty ()) {
      const cell_t * const parent = q.front ().parent;
      const int min_ants_cnt = q.front ().min_ants_cnt;
      q.pop ();
      for (cell_t * const next_cell : parent->neighs ()) {
        if (next_cell->ants (player.id ()) <= 0)
          continue;
        const int new_min_ants_cnt = std::min (next_cell->ants (player.id ()), min_ants_cnt);
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
      cell->set_ants_chain_power (player.id (), min_ants_cnt);
    }
  }
  //for (const cell_t &cell : m_cells)
  //  if (cell.ants_chain_power (0) + cell.ants_chain_power (1) > 0)
  //    std::cerr << cell.id () << ": " << cell.ants_chain_power (0) << " vs " << cell.ants_chain_power (1) << std::endl;
}
void game_t::fill_beacons () {
  player_t &iam = m_players[0];
  player_t &enemy = m_players[1];
  std::unordered_set<cell_t*> path;
  std::vector<std::unordered_set<cell_t*>> lines;
  std::unordered_set<cell_t*> new_line;
  // TO-DO: идти к тем кристалам что ближе к врагу, те что далеко можно будет собрать в последний момент
  // TO-DO: выдавать "персональные" задания каждому муравью прям на 1 клеточку чтобы шли как надо
  // TO-DO: полностью поменять систему выбора куда бежать, добавить атаку на чужие клетки

  for (cell_t * const base_cell : iam.bases ()) {
    path.insert (base_cell);
    base_cell->set_chain_parent (nullptr);
  }

  std::unordered_set<cell_t *> all_aim_cells (m_all_aim_cells.begin (), m_all_aim_cells.end ());
  struct temp_data_t {
    int min_dis, val_cnt;
    cell_t * const parent;
  };
  std::unordered_map<cell_t *, temp_data_t> candidates;

  while (!all_aim_cells.empty () && iam.ants_cnt_free () > 0) {
    candidates.clear ();

    for (const auto &path_cells : {path, new_line})
      for (cell_t * const cell_from : path_cells) {
        for (cell_t * const next_cell : cell_from->neighs ())
          if (!path.count (next_cell) && !new_line.count (next_cell)) {
            const auto it_ins = candidates.insert ({next_cell, temp_data_t {INF, 0, cell_from}});
            if (!it_ins.second)
              continue;
            const cell_t * const new_cell = it_ins.first->first;
            temp_data_t &new_cell_data = it_ins.first->second;
            for (const cell_t * const aim_cell : all_aim_cells) {
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
    if (new_line.empty ())
      new_line.insert (best_cond_it->second.parent);
    new_line.insert (add_cell);
    if (all_aim_cells.erase (add_cell) > 0) {
      const int line_beacon_per_cell = 1 + (*std::max_element (
          new_line.begin (), new_line.end (),
          [&enemy] (const cell_t * const a, const cell_t * const b) {
            return a->ants_chain_power (enemy.id ()) < b->ants_chain_power (enemy.id ()); }
        ))->ants_chain_power (enemy.id ());
      int ants_need_to_use = 0;
      for (const cell_t * const cell : new_line)
        if (cell->beacon () < line_beacon_per_cell)
          ants_need_to_use += line_beacon_per_cell - cell->beacon ();

      if (ants_need_to_use > iam.ants_cnt_free ())
        continue;

      for (cell_t * const cell : new_line)
        set_min_beacon (*cell, line_beacon_per_cell);

      path.insert (new_line.begin (), new_line.end ());
      lines.emplace_back (new_line);
      new_line.clear ();
    }
    //std::cerr << "# " << add_cell->id () << " " << best_cond_it->second.min_dis << " " << best_cond_it->second.val_cnt << std::endl;
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
