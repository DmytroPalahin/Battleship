/* SPDX-License-Identifier: MIT */
/**
 * @file app.h
 * @brief Application shell: command line, lifetime of the model and controller.
 */
#ifndef BATTLESHIP_UI_APP_H
#define BATTLESHIP_UI_APP_H

/**
 * Runs the application until its window is closed.
 * @return the process exit status.
 */
int bs_app_run(int argc, char **argv);

#endif /* BATTLESHIP_UI_APP_H */
