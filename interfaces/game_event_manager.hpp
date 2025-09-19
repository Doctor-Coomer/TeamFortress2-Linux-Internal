#ifndef GAME_EVENT_MANAGER_HPP
#define GAME_EVENT_MANAGER_HPP

class IGameEventListener;

class GameEvent
{
public:
  virtual ~GameEvent() {};
  virtual const char* get_name() const = 0;
  virtual bool is_reliable() const = 0;
  virtual bool is_local() const = 0;
  virtual bool is_empty(const char* keyName = nullptr) = 0;
  virtual bool get_bool(const char* keyName = nullptr, bool defaultValue = false) = 0;
  virtual int get_int(const char* keyName = nullptr, int defaultValue = 0) = 0;
  virtual float get_float(const char* keyName = nullptr, float defaultValue = 0.0f) = 0;
  virtual const char* get_string(const char* keyName = nullptr, const char* defaultValue = "") = 0;
  virtual void set_bool(const char* keyName, bool value) = 0;
  virtual void set_int(const char* keyName, int value) = 0;
  virtual void set_float(const char* keyName, float value) = 0;
  virtual void set_string(const char* keyName, const char* value) = 0;
};

class BaseInterface {
public:
  virtual ~BaseInterface(void) {};
};

class GameEventManager : public BaseInterface {
public:
  virtual ~GameEventManager(void) {};
  virtual int load_events_from_file(const char* filename) = 0;
  virtual void reset() = 0;
  virtual bool add_listener(IGameEventListener* listener, const char* name, bool bServerSide) = 0;
  virtual bool find_listener(IGameEventListener* listener, const char* name) = 0;
  virtual void remove_listener(IGameEventListener* listener) = 0;
  virtual GameEvent* create_event(const char* name, bool force = false) = 0;
  virtual bool fire_event(GameEvent* event, bool dont_broadcast = false) = 0;
  virtual bool fire_event_client_side(GameEvent* event) = 0;
  virtual GameEvent* duplicate_event(GameEvent* event) = 0;
  virtual void free_event(GameEvent* event) = 0;
  virtual bool serialize_event(GameEvent* event, void* buf) = 0;
  virtual GameEvent* unserialize_event(void* buf) = 0;
};

inline static GameEventManager* game_event_manager;

#endif
