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
  
class cell_t {
public:
  void init (const int id);
  void read ();
  inline int id () const { return m_id; }
  inline int beacon () const { return m_beacon; }
  inline const std::array<int, NEIGH_SIZE> &neighs () const { return m_neigh; }
  inline bool is_egg () const { return m_type == 1 && m_resources > 0; }
  inline int resources () const { return m_resources; }
  inline int ants (const int player_id) { return m_ants[player_id]; }
  inline void add_beacon (const int val = 1) { m_beacon += val; }
private:
  int m_id = INVALID_ID;
  int m_type = -1;
  int m_resources = -1;
  int m_beacon = -1;
  std::array<int, NEIGH_SIZE> m_neigh;
  std::array<int, PLAYER_SIZE> m_ants;
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
  void update_step (std::vector<cell_t> &cells);
  inline const std::vector<cell_t*> &bases () const { return m_bases; }
  inline cell_t &base () const { return *m_bases.front (); }
  inline int ants_cnt () const { return m_ants_cnt; }
private:
  int m_id = INVALID_ID;
  std::vector<cell_t*> m_bases;

  int m_ants_cnt;
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

  void fill_bacons ();

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

void cell_t::init (const int id) {
  m_id = id;
  std::cin >> m_type >> m_resources; std::cin.ignore ();
  for (int &neigh_id : m_neigh)
    std::cin >> neigh_id, std::cin.ignore ();
}
void cell_t::read () {
  std::cin >> m_resources >> m_ants[0] >> m_ants[1]; std::cin.ignore ();
  m_beacon = 0;
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
      for (const int neigh_id : cell_to.neighs ())
        if (neigh_id != INVALID_ID && m_dist[cell_from.id ()][neigh_id] > dist + 1)
          q.push ({m_cells[neigh_id], dist + 1});
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
}

game_t::game_t () {
  int number_of_cells;
  std::cin >> number_of_cells; std::cin.ignore ();
  m_cells.resize (number_of_cells);
  for (int i_cell = 0; i_cell < number_of_cells; ++i_cell)
    m_cells[i_cell].init (i_cell);
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
  for (cell_t &cell : m_cells) {
    cell.read ();
    if (cell.resources () > 0)
      m_aims.push_back (&cell);
  }
  for (player_t &player : m_players)
    player.update_step (m_cells);
}
void game_t::play_step () {
  std::vector<const cell_t *> cells;
  // int cell_id_from = m_players[0].base ().id ();
  // for (const cell_t &cell : m_cells)
  //   if (cell.resources () > 0)
  //     {
  //       commit_line (cell_id_from, cell.id (), 1);
  //       //cell_id_from = cell.id ();
  //     }
  fill_bacons ();
  //for (const cell_t &cell : m_cells)
  //  if (cell.beacon () > 0)
  //    commit_beacon (cell.id (), cell.beacon ());
  commit_wait ();
  stop_step_timer ();
  std::cout << m_actions_text << std::endl;
}
void game_t::fill_bacons () {
  std::unordered_set<cell_t*> path;
  for (cell_t * const base_cell : m_players[0].bases ())
    path.insert (base_cell), commit_beacon (base_cell->id (), 1);;

  std::unordered_set<cell_t *> aims (m_aims.begin (), m_aims.end ());
  struct temp_data_t { int min_dis, val_cnt; };
  std::unordered_map<cell_t *, temp_data_t> candidates;

  while (!aims.empty () && static_cast<int> (path.size ()) < m_players[0].ants_cnt ()) {
    candidates.clear ();
    for (const cell_t * const cell_from : path)
      for (const int neigh_id : cell_from->neighs ())
        if (neigh_id != INVALID_ID && !path.count (&m_cells[neigh_id])) {
          const auto it_ins = candidates.insert ({&m_cells[neigh_id], temp_data_t {INF, 0}});
          if (!it_ins.second)
            continue;
          const cell_t * const new_cell = it_ins.first->first;
          temp_data_t &new_cell_val = it_ins.first->second;
          for (const cell_t * const aim_cell : aims) {
            const int dist_to_aim = dist (new_cell->id (), aim_cell->id ());
            if (new_cell_val.min_dis > dist_to_aim)
              new_cell_val = temp_data_t {dist_to_aim, aim_cell->resources ()};
            else if (new_cell_val.min_dis == dist_to_aim)
              new_cell_val.val_cnt += aim_cell->resources ();
          }
    }   
    if (candidates.empty ())
      break;
    const auto &add_cell = std::min_element (candidates.begin (), candidates.end (),
      [this] (const auto &a, const auto &b) {
        if (a.second.min_dis != b.second.min_dis)
          return a.second.min_dis < b.second.min_dis;
        return a.second.val_cnt > b.second.val_cnt;
      });
    path.insert (add_cell->first);
    aims.erase (add_cell->first);
    //std::cerr << "# " << add_cell->first->id () << " " << add_cell->second.min_dis << " " << add_cell->second.val_cnt << std::endl;
    commit_beacon (add_cell->first->id (), 1);
  }

  for (cell_t * const cell : path)
    cell->add_beacon ();
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
