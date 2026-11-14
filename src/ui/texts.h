/* SPDX-License-Identifier: MIT */
/**
 * @file texts.h
 * @brief Every string shown by the GTK interface, in one place.
 *
 * The game speaks French, the language of the school it was written for.
 * Keeping the wording here, away from the widget code, is what would make a
 * translation (gettext) a mechanical change.
 *
 * This file is a hand-laid-out table of text: automatic formatting is turned
 * off, because the display width of emoji is not something tools agree on.
 */
#ifndef BATTLESHIP_UI_TEXTS_H
#define BATTLESHIP_UI_TEXTS_H

/* clang-format off */

#define BS_TEXT_WINDOW_TITLE "BATAILLE NAVALE"

/* ------------------------------------------------------------------- menu */

#define BS_TEXT_MENU_TITLE     "⬇️▶️  CHOISIR LE NIVEAU  ▶️⬇️"
#define BS_TEXT_RULES_BUTTON   "📜  RÈGLES  📜"
#define BS_TEXT_TRAILER_BUTTON "👀  BANDE – ANNONCE  👀"
#define BS_TEXT_LEVEL_EASY     "😜  FACILE  😜"
#define BS_TEXT_LEVEL_MEDIUM   "😐  MOYEN  😐"
#define BS_TEXT_LEVEL_HARD     "🤔  DIFFICILE  🤔"

/* ------------------------------------------------------------------- game */

#define BS_TEXT_FLEET_PLAYER     "🇫🇷  SUP GALILÉE MARINE NATIONALE  🇫🇷"
#define BS_TEXT_FLEET_ENEMY      "🏴‍☠️  FLOTTE DE PIRATES  🏴‍☠️"
#define BS_TEXT_LEVEL_HEADING    "↘️  LE NIVEAU DE DIFFICULTÉ  ↙️"
#define BS_TEXT_CONTROL_HEADING  "🎬  GESTION DU JEU  🎬"
#define BS_TEXT_RESTART_BUTTON   "🔄  REDÉMARRER LE JEU  🔄"
#define BS_TEXT_RESHUFFLE_BUTTON "⚓  REPLACER LA FLOTTE  ⚓"
#define BS_TEXT_QUIT_BUTTON      "🚪  SORTIR  🚪"

#define BS_TEXT_STATUS_READY \
    "À vous l'honneur, Capitaine !\n" \
    "Choisissez une case de la flotte pirate."
#define BS_TEXT_STATUS_HUMAN_TURN    "À vous de jouer, Capitaine !"
#define BS_TEXT_STATUS_COMPUTER_TURN "Les pirates ripostent…"

/* printf-style: shots, hits, ships sunk, fleet size. */
#define BS_TEXT_SCORE_FORMAT "Tirs : %u   ·   Touchés : %u   ·   Navires coulés : %u / %d"

/* ------------------------------------------------------------ end of game */

#define BS_TEXT_VICTORY_TITLE "Victoire !"
#define BS_TEXT_VICTORY_DIALOG \
    "Bravo ✌️🎉 ! Mission accomplie ✅ : vous avez protégé la science contre les pirates ! " \
    "Vous êtes un bon 🎖️ Commandant 🎖️ de la Sup Galilée Marine Nationale 🇫🇷. " \
    "Continuez comme ça, Capitaine 🤝💪 !"
#define BS_TEXT_VICTORY_STATUS \
    "🏅  Cher Capitaine, gagner, c'est génial.\n" \
    "Les guerres se gagnent par de petites victoires ✌️ !\n" \
    "Mais trop de jeux peuvent être dangereux pour la santé :\n" \
    "nous vous conseillons de faire une pause 🏁. À bientôt ! 👋"

#define BS_TEXT_DEFEAT_TITLE "Défaite"
#define BS_TEXT_DEFEAT_DIALOG \
    "Malheureusement, la mission n'a pas été accomplie ❌. Mais ce n'est pas grave : " \
    "la prochaine fois, nous les battrons à coup sûr 💪 !"
#define BS_TEXT_DEFEAT_STATUS \
    "Cher Capitaine, perdre n'est pas la fin du monde 🏋️.\n" \
    "Vous avez perdu une bataille, mais pas la guerre 💪 !\n" \
    "Préparez-vous mieux 📙 et retentez votre chance 🥊.\n" \
    "À bientôt ! 👋"

#define BS_TEXT_REPLAY_BUTTON "Rejouer"
#define BS_TEXT_MENU_BUTTON   "Changer de niveau"
#define BS_TEXT_LEAVE_BUTTON  "Quitter"

/* ------------------------------------------------------------------ rules */

#define BS_TEXT_RULES_TITLE "📖  RÈGLES DU JEU  📖"

/* Initialiser of an array of strings: one entry per paragraph. */
#define BS_TEXT_RULES_LINES { \
    "➡️ Chaque joueur possède une flotte de 10 bateaux 🚢 :", \
    "🔻 1 case — 4 bateaux      🔻 2 cases — 3 bateaux\n" \
    "🔻 3 cases — 2 bateaux    🔻 4 cases — 1 bateau", \
    "➡️ Tous ces bateaux ⛵ sont placés sur une grille carrée de 10 cases sur 10.", \
    "➡️ Ils ne peuvent pas se toucher 🚫, ni par les côtés, ni par les coins.", \
    "➡️ L'objectif est de détruire 🔥 tous les bateaux de l'adversaire. " \
    "Le premier joueur à y parvenir remporte la partie ✌️.", \
    "➡️ Lorsqu'un tir touche un bateau ennemi, une explosion 💥 marque la case " \
    "et le joueur tire à nouveau.", \
    "➡️ En cas d'échec, la case est marquée d'un impact et le tour passe à l'adversaire.", \
    "➡️ Un drapeau 🚩 signale une case où aucun bateau ne peut se trouver.", \
    "➡️ Le jeu se termine lorsque l'un des joueurs a perdu tous ses bateaux ❌🛳️❌.", \
}

/* ---------------------------------------------------------------- trailer */

#define BS_TEXT_TRAILER_TITLE "📢🎥  BANDE – ANNONCE  🎥📢"

/* Initialiser of an array of strings: one entry per paragraph. */
#define BS_TEXT_TRAILER_LINES { \
    "Nous vous présentons 🚀⛵💥 BATAILLE NAVALE 💥⛵🚀 — le jeu bien-aimé et célèbre " \
    "de notre enfance 👶, joué dans les cahiers 📝.", \
    "Il a été, est et restera le meilleur jeu de classe avec notre voisin de pupitre.", \
    "BATAILLE NAVALE est un jeu de stratégie classique pour 2️⃣ joueurs, où vous êtes " \
    "👩‍✈️ Capitaine 👨‍✈️ de la 🇫🇷 SUP GALILÉE MARINE NATIONALE 🇫🇷 !", \
    "Vous devez découvrir la position des navires 🛳️ de vos adversaires, les ☠️ Pirates ☠️, " \
    "avant qu'ils ne découvrent les vôtres et ne coulent votre flotte.", \
    "Tous les 👩‍🎓 étudiants 👨‍🎓, ingénieurs et 🧑‍🏫 enseignants de la SUP GALILÉE se " \
    "trouvent dans une situation très dangereuse ⚠️ : ils sont attaqués par les Pirates.", \
    "Prenez le commandement de votre 🚀 Armada 🚀 et détruisez la flottille ennemie grâce à " \
    "votre intuition, votre stratégie et surtout vos compétences d'ingénierie.", \
    "Développez votre propre tactique pour remporter la victoire sur les trois niveaux " \
    "de difficulté !", \
    "Affrontez les 🏴‍☠️ Pirates 🏴‍☠️ et prouvez que vous êtes un 💯 Véritable Ingénieur 💯 " \
    "de la SUP GALILÉE 😎✌️ !", \
    "⚔️ Que le meilleur l'emporte ⚔️ et VIVE SUP GALILÉE 🦾🔝✌️ !", \
}

/* clang-format on */

#endif /* BATTLESHIP_UI_TEXTS_H */
