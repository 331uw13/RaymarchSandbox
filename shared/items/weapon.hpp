#ifndef AMBIENT3D_WEAPON_HPP
#define AMBIENT3D_WEAPON_HPP

namespace AM {

    namespace FireMode {
        static constexpr int SEMI_AUTO = 1<<0;
        static constexpr int FULL_AUTO = 1<<1;
    };

    struct WeaponStruct {

        float   accuracy;
        float   base_damage;
        float   recoil;
        float   firerate;
        int     firemode;

    };
};


#endif
