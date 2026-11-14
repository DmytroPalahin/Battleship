/* SPDX-License-Identifier: MIT */
/**
 * @file resources.h
 * @brief Identifiers shared by the modules of the GTK application.
 *
 * Images, sounds and the style sheet are compiled into the executable as a
 * GResource bundle, so the program runs from any directory and installs as a
 * single file.
 */
#ifndef BATTLESHIP_UI_RESOURCES_H
#define BATTLESHIP_UI_RESOURCES_H

/** Reverse-DNS identifier of the application. */
#define BS_APPLICATION_ID "io.github.dmytropalahin.Battleship"

/** Root of the bundled resources. */
#define BS_RESOURCE_PREFIX "/io/github/dmytropalahin/battleship"

#endif /* BATTLESHIP_UI_RESOURCES_H */
