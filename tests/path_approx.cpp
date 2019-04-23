#include "path.h"

#include <gtest/gtest.h>

TEST(path_set_apply_approximation, simplify_short) {
    lhecker_bachelor::path::set actual{
        {
            {0, 0, 1},
        },
        {
            {0, 0, 1},
            {0, 0, 1},
        },
    };
    lhecker_bachelor::path::set expected{
        {
            {0, 0, 1},
        },
        {
            {0, 0, 1},
        },
    };

    actual.apply_approximation(1);
    EXPECT_EQ(actual, expected);
}

TEST(path_set_apply_approximation, closed) {
    // (0,0) (8,0)
    //   ●─────●
    //   │    ╱
    //   │   ╱
    //   │  + (4,4)
    //   │ ╱
    //   │╱
    //   ●
    // (0,8)
    lhecker_bachelor::path::set actual{
        {
            {0, 0, 1},
            {8, 0, 1},
            {4, 4, 1},
            {0, 8, 1},
            {0, 0, 1},
        },
    };
    lhecker_bachelor::path::set expected{
        {
            {0, 0, 1},
            {8, 0, 1},
            {0, 8, 1},
            {0, 0, 1},
        },
    };

    actual.apply_approximation(1);
    EXPECT_EQ(actual, expected);
}

TEST(path_set_apply_approximation, within_error) {
    lhecker_bachelor::path::set actual{
        {
            {0, 0, 1}, // '  (●)  '
            {0, 1, 2}, // ' ( ● ) '
            {0, 2, 3}, // '(  ●  )'
            {0, 3, 1}, // '  (●)  '
        },
    };
    lhecker_bachelor::path::set expected{
        {
            {0, 0, 1},
            {0, 3, 1},
        },
    };

    actual.apply_approximation(4);
    EXPECT_EQ(actual, expected);
}

TEST(path_set_apply_approximation, beyond_error) {
    lhecker_bachelor::path::set actual{
        {
            {0, 0, 1}, //  ' (●) '
            {0, 1, 2}, //  '( ● )'
            {0, 2, 3}, //  (  ●  )
            {0, 3, 1}, //  ' (●) '
        },
    };
    lhecker_bachelor::path::set expected{
        {
            {0, 0, 1},
            {0, 2, 3},
            {0, 3, 1},
        },
    };

    actual.apply_approximation(3.9);
    EXPECT_EQ(actual, expected);
}

TEST(path_set_apply_approximation, within_displacement) {
    lhecker_bachelor::path::set actual{
        {
            {0, 0, 3}, // (  ●  )
            {2, 1, 1}, //   (  ●  )
            {0, 2, 3}, // (  ●  )
        },
    };
    lhecker_bachelor::path::set expected{
        {
            {0, 0, 3},
            {0, 2, 3},
        },
    };

    actual.apply_approximation(4);
    EXPECT_EQ(actual, expected);
}

TEST(path_set_apply_approximation, beyond_displacement) {
    lhecker_bachelor::path::set actual{
        {
            {0, 0, 3}, // (  ●  )
            {3, 1, 1}, //    (  ●  )
            {0, 2, 3}, // (  ●  )
        },
    };
    lhecker_bachelor::path::set expected{
        {
            {0, 0, 3},
            {3, 1, 1},
            {0, 2, 3},
        },
    };

    actual.apply_approximation(2);
    EXPECT_EQ(actual, expected);
}

TEST(path_set_apply_approximation, within_narrowing) {
    lhecker_bachelor::path::set actual{
        {
            {0, 0, 3}, // (  ●  )
            {2, 1, 1}, //     (●)
            {0, 2, 3}, // (  ●  )
        },
    };
    lhecker_bachelor::path::set expected{
        {
            {0, 0, 3},
            {2, 1, 1},
            {0, 2, 3},
        },
    };

    actual.apply_approximation(1);
    EXPECT_EQ(actual, expected);
}

TEST(path_set_apply_approximation, beyond_narrowing) {
    lhecker_bachelor::path::set actual{
        {
            {0, 0, 3}, // (  ●  )
            {2, 1, 1}, //     (●)
            {0, 2, 3}, // (  ●  )
        },
    };
    lhecker_bachelor::path::set expected{
        {
            {0, 0, 3},
            {2, 1, 1},
            {0, 2, 3},
        },
    };

    actual.apply_approximation(1);
    EXPECT_EQ(actual, expected);
}
