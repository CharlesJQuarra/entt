#ifndef LONG_LIVED_VERSION_ENTITY_H
#define LONG_LIVED_VERSION_ENTITY_H


#include <type_traits>

namespace entt {


  struct LongLivedVersionIdType {
    const LongLivedVersionIdType* prev;
    mutable LongLivedVersionIdType* next;
    using refcount_type = std::uint32_t;
    mutable refcount_type refcount;

    //constructors should be private and only instantiable by friend entity_type
    LongLivedVersionIdType() : prev(nullptr), next(nullptr), refcount(0) {}
    LongLivedVersionIdType(const LongLivedVersionIdType* p) : prev(p), next(nullptr), refcount(0) {}

    static LongLivedVersionIdType* getRoot() {
      static LongLivedVersionIdType root;
      return &root;
    }

    static LongLivedVersionIdType*& getHead() {
      static LongLivedVersionIdType* head;
      return head;
    }

    ~LongLivedVersionIdType() {
      //assert(refcount == 0);
      if (prev != nullptr)
        prev->adjust_next(next);
      if (next != nullptr)
        next->prev = prev;
    }

    void adjust_next(LongLivedVersionIdType* nnext) const {
      next = nnext;
    }

    void decref() {
      assert(refcount > 0);
      refcount--;
      bool firstNode = (this->prev == nullptr);
      if (0 == refcount && !firstNode)
        delete this;
    }

    void incref() {
      refcount++;
    }

    const LongLivedVersionIdType* id() const { return this; }
    inline bool operator==(const LongLivedVersionIdType* other) const {
      return this == other;
    };

    inline bool operator!=(const LongLivedVersionIdType* other) const {
      return this != other;
    }

    LongLivedVersionIdType* upgrade_basic() {
      LongLivedVersionIdType* pnext = this->next;
      if (pnext == nullptr) {
        pnext = new LongLivedVersionIdType(this);
        this->next = pnext;
        pnext->incref();
      }
      this->decref();
      return pnext;
    }

    template< int LookAhead = 3, typename = void >
    LongLivedVersionIdType* upgrade_lookahead() {
      LongLivedVersionIdType* pnext[LookAhead];
      int idx = 0;
      unsigned int maxRefCount = 0;
      LongLivedVersionIdType* maxRefCountPtr = nullptr;
      LongLivedVersionIdType* pthis = this;
      LongLivedVersionIdType* pprev = nullptr;
      do {
        pprev = pthis;
        pnext[idx++] = pthis = pthis->next;
        if (pthis && pthis->refcount > maxRefCount) {
          maxRefCount = pthis->refcount;
          maxRefCountPtr = pthis;
        }
      } while (pthis && idx < LookAhead);
      if (maxRefCountPtr == nullptr) {
        pthis = new LongLivedVersionIdType(pprev);
        pprev->next = pthis;
        pthis->incref();
      } else {
        pthis = maxRefCountPtr;
        maxRefCountPtr->incref();
      }
      this->decref();
      return pthis;
    }
  };

  struct EntTypeWithLongTermVersionId {
    using entity_type = std::uint32_t;
    using version_type = std::add_pointer_t<LongLivedVersionIdType>;

    entity_type entity_id;
    version_type version_id;
    static constexpr version_type version_mask = nullptr;
    static const version_type default_version() {
      return LongLivedVersionIdType::getRoot();
    }

    EntTypeWithLongTermVersionId(): entity_id(), version_id(nullptr) {}

    EntTypeWithLongTermVersionId(const entity_type e_id, const version_type v_id) : entity_id(e_id), version_id(v_id) {
      //assert(v_id != nullptr);
      if (version_id != nullptr)
        version_id->incref();
    }

    inline bool operator==(const EntTypeWithLongTermVersionId& other) const {
      return entity_id == other.entity_id && version_id == other.version_id;
    }
    inline bool operator!=(const EntTypeWithLongTermVersionId& other) const {
      return entity_id != other.entity_id || version_id != other.version_id;
    }
    inline bool operator<(const EntTypeWithLongTermVersionId& other) const {
      if (entity_id < other.entity_id)
        return true;
      return (version_id < other.version_id);
    }
    inline EntTypeWithLongTermVersionId& operator=(const EntTypeWithLongTermVersionId& other) {
      entity_id = other.entity_id;
      version_id = other.version_id;
      return *this;
    }

    ~EntTypeWithLongTermVersionId() {
      if (nullptr != version_id) {
        version_id->decref();
      }
    }
  };


}

#endif