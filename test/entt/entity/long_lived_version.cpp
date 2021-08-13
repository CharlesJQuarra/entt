

#include <functional>
#include <type_traits>
#include <gtest/gtest.h>
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

TEST(LongLivedVersion, Traits) {
    using traits_type = entt::entt_traits<entt::entity>;
    entt::registry registry{};

    registry.destroy(registry.create());
    const auto entity = registry.create();
    const auto other = registry.create();

    ASSERT_EQ(entt::to_integral(entity), traits_type::to_integral(entity));
    ASSERT_NE(entt::to_integral(entity), entt::to_integral<entt::entity>(entt::null));
    ASSERT_NE(entt::to_integral(entity), entt::to_integral(entt::entity{}));

    ASSERT_EQ(traits_type::to_entity(entity), 0u);
    ASSERT_EQ(traits_type::to_version(entity), 1u);
    ASSERT_EQ(traits_type::to_entity(other), 1u);
    ASSERT_EQ(traits_type::to_version(other), 0u);

    ASSERT_EQ(traits_type::construct(traits_type::to_entity(entity), traits_type::to_version(entity)), entity);
    ASSERT_EQ(traits_type::construct(traits_type::to_entity(other), traits_type::to_version(other)), other);
    ASSERT_NE(traits_type::construct(traits_type::to_entity(entity), {}), entity);

    ASSERT_EQ(traits_type::construct(), entt::tombstone | static_cast<entt::entity>(entt::null));
    ASSERT_EQ(traits_type::construct(), entt::null | static_cast<entt::entity>(entt::tombstone));

    ASSERT_EQ(traits_type::construct(), static_cast<entt::entity>(entt::null));
    ASSERT_EQ(traits_type::construct(), static_cast<entt::entity>(entt::tombstone));
    ASSERT_EQ(traits_type::construct(), entt::entity{~entt::id_type{}});
}

TEST(LongLivedVersion, BasicLinkedList) {

}
