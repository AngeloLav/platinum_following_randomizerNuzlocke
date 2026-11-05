# Delta Platinum

Decompilation of Pokémon Platinum with HeartGold-style follower Pokémon and unique colorations for every individual mon.

This is a fork of [pret/pokeplatinum](https://github.com/pret/pokeplatinum), the sister project of
[Delta Heart Gold](https://github.com/dusknight4/deltaheartgold). It is published as **three commits on top of
upstream**, so you can take only the part you want:

1. **Followers** – base game plus HeartGold/SoulSilver-style follower Pokémon: every HG follower sprite and form,
   large Pokémon, talking to your follower, the Poké Ball release, and following through cutscenes. It uses plain
   normal and shiny palettes and has no gameplay changes. See [docs/FOLLOWER_POKEMON.md](docs/FOLLOWER_POKEMON.md).
2. **Colour mod** – adds the personality-based hue/saturation/value rotation in every location (sprites, battles,
   summary, PC, trades, followers). See [docs/COLOR_MOD.md](docs/COLOR_MOD.md).
3. **Everything else** – the full Delta Platinum mod: custom types from IVs, ability donors, HeartGold data sync,
   per-type level-up moves, trainer AI changes, the Arceus transform, battle/item/breeding changes, the R4 / YSMenu
   fix, follower improvements made after commit 1, and other fixes.

**This repository contains no ROMs, saves, compiler or SDK files.** Build it exactly like upstream, as described in
[INSTALL.md](INSTALL.md): the build downloads the toolchain and SDK subprojects itself. The ROM this builds is a
*modified* game, so it will not match the upstream sha1s below.

---

# Pokémon Platinum

This is a WIP decompilation of Pokémon Platinum. For instructions on how to set up the repository, please read [`INSTALL.md`](INSTALL.md).

If you are interested in contributing, please read [`CONTRIBUTING.md`](/CONTRIBUTING.md).

This repository builds the following ROMs:

* [**pokeplatinum.us.nds**](https://datomatic.no-intro.org/index.php?page=show_record&s=28&n=4997) (Rev 1): `sha1: 0862ec35b24de5c7e2dcb88c9eea0873110d755c`
* [**pokeplatinum.us.nds**](https://datomatic.no-intro.org/index.php?page=show_record&s=28&n=3541) (Rev 0): `sha1: ce81046eda7d232513069519cb2085349896dec7`

For contacts and other pret projects, see [pret.github.io](https://pret.github.io/).
