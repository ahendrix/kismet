/*
    This file is part of Kismet

    Kismet is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kismet is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Kismet; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "config.h"

#include <math.h>

#include "panelfront.h"
#include "displaynetworksort.h"

#if (defined(HAVE_LIBNCURSES) && defined(HAVE_LIBPANEL) && defined(BUILD_PANEL))

int PanelFront::MainInput(void *in_window, int in_chr) {
    kis_window *kwin = (kis_window *) in_window;
    char sendbuf[1024];

    switch (in_chr) {
    case 'Q':
        return FE_QUIT;
        break;
    case 'q':
        WriteStatus("Use capital-Q to quit Kismet.");
        break;
    case KEY_UP:
        if (sortby != sort_auto) {
            if (kwin->selected == 0 && kwin->start != 0) {
                kwin->start--;
            } else if (kwin->selected > 0) {
                kwin->selected--;
            }
        } else {
            WriteStatus("Cannot scroll in autofit sort mode.");
        }

        break;
    case KEY_DOWN:
        if (sortby != sort_auto) {
            if (kwin->start + kwin->selected < last_draw_size - 1) {
                if ((kwin->start + kwin->selected >= kwin->end) &&
                    (kwin->start + kwin->selected + 1 < last_draw_size))
                    kwin->start++;
                else
                    kwin->selected++;
            }

        } else {
            WriteStatus("Cannot scroll in autofit sort mode.");
        }
        break;
    case KEY_RIGHT:
    case '+':
        if (sortby != sort_auto && last_displayed.size() > 0) {
            if (last_displayed[kwin->selected]->type == group_bundle)
                last_displayed[kwin->selected]->expanded = 1;
        } else {
            WriteStatus("Cannot expand groups in autofit sort mode.");
        }
        break;
    case KEY_LEFT:
    case '-':
        if (sortby != sort_auto && last_displayed.size() > 0) {
            if (last_displayed[kwin->selected]->type == group_bundle)
                last_displayed[kwin->selected]->expanded = 0;
        } else {
            WriteStatus("Cannot collapse groups in autofit sort mode.");
        }
        break;
    case 'i':
    case 'I':
    case KEY_ENTER:
        if (sortby != sort_auto &&  last_displayed.size() > 0) {
            details_network = last_displayed[kwin->selected];
            SpawnWindow("Network Details",
                        &PanelFront::DetailsPrinter, &PanelFront::DetailsInput);
        } else {
            WriteStatus("Cannot view details in autofit sort mode.");
        }
        break;
    case 't':
    case 'T':
        if (sortby != sort_auto && last_displayed.size() > 0) {
            if (last_displayed[kwin->selected]->tagged)
                last_displayed[kwin->selected]->tagged = 0;
            else
                last_displayed[kwin->selected]->tagged = 1;
        } else {
            WriteStatus("Cannot tag networks in autofit sort mode.");
        }
        break;
    case 'n':
    case 'N':
        if (sortby != sort_auto && last_displayed.size() > 0) {
            details_network = last_displayed[kwin->selected];
            SpawnWindow("Group Name", &PanelFront::GroupNamePrinter, NULL, 3, 30);
        } else {
            WriteStatus("Cannot name groups in autofit sort mode.");
        }
        break;
    case 'g':
    case 'G':
        if (sortby != sort_auto &&  last_displayed.size() > 0) {
            details_network = GroupTagged();
            if (details_network != NULL)
                SpawnWindow("Group Name", &PanelFront::GroupNamePrinter, NULL, 3, 30);
        } else {
            WriteStatus("Cannot create groups in autofit sort mode.");
        }
        break;
    case 'u':
    case 'U':
        if (sortby != sort_auto && last_displayed.size() > 0) {
            if (last_displayed[kwin->selected] != NULL)
                DestroyGroup(last_displayed[kwin->selected]);
        } else {
            WriteStatus("Cannot ungroup in autofit sort mode.");
        }
        break;
    case 'h':
    case 'H':
        if (kwin->win->_maxx < 64)
            SpawnHelp(KismetHelpTextNarrow);
        else
            SpawnHelp(KismetHelpText);
        //SpawnPopup("Kismet Help", &PanelFront::PrintKismetHelp, HELP_SIZE);
        break;
    case 'z':
    case 'Z':
        ZoomNetworks();
        break;
    case 's':
    case 'S':
        SpawnWindow("Sort Network", &PanelFront::SortPrinter, &PanelFront::SortInput, SORT_SIZE);
        break;
    case 'l':
    case 'L':
        SpawnWindow("Wireless Card Power", &PanelFront::PowerPrinter, &PanelFront::PowerInput, 3);
        break;
    case 'd':
    case 'D':
        snprintf(sendbuf, 1024, "!%u strings \n", (unsigned int) time(0));
        client->Send(sendbuf);
        WriteStatus("Requesting strings from the server");

        SpawnWindow("Data Strings Dump", &PanelFront::DumpPrinter, &PanelFront::DumpInput);
        break;
    case 'r':
    case 'R':
        SpawnWindow("Packet Rate", &PanelFront::RatePrinter, &PanelFront::RateInput);
        break;
    case 'w':
    case 'W':
        SpawnWindow("Alerts", &PanelFront::AlertPrinter, &PanelFront::AlertInput);
        break;
    case 'a':
    case 'A':
        SpawnWindow("Statistics", &PanelFront::StatsPrinter, &PanelFront::StatsInput, 18, 65);
        break;
    case 'p':
    case 'P':
        snprintf(sendbuf, 1024, "!%u packtypes \n", (unsigned int) time(0));
        client->Send(sendbuf);
        WriteStatus("Requesting packet types from the server");

        SpawnWindow("Packet Types", &PanelFront::PackPrinter, &PanelFront::PackInput);
        break;
    case 'f':
    case 'F':
        if (sortby != sort_auto && last_displayed.size() > 0) {
            details_network = last_displayed[kwin->selected];
            SpawnWindow("Network Location", &PanelFront::GpsPrinter, &PanelFront::GpsInput, 8, 34);
        } else {
            WriteStatus("Cannot view network GPS info in autofit sort mode.");
        }
        break;
    case 'm':
    case 'M':
        MuteToggle();
        break;
    }

    return 1;
}

int PanelFront::SortInput(void *in_window, int in_chr) {
    switch (in_chr) {
    case 'a':
    case 'A':
        sortby = sort_auto;
        WriteStatus("Autofitting network display");
        break;
    case 'c':
    case 'C':
        sortby = sort_channel;
        WriteStatus("Sorting by channel");
        break;
    case 'f':
        sortby = sort_first;
        WriteStatus("Sorting by time first detected");
        break;
    case 'F':
        sortby = sort_first_dec;
        WriteStatus("Sorting by time first detected (descending)");
        break;
    case 'l':
        sortby = sort_last;
        WriteStatus("Sorting by time most recently active");
        break;
    case 'L':
        sortby = sort_last_dec;
        WriteStatus("Sorting by time most recently active (descending)");
        break;
    case 'b':
        sortby = sort_bssid;
        WriteStatus("Sorting by BSSID");
        break;
    case 'B':
        sortby = sort_bssid_dec;
        WriteStatus("Sorting by BSSID (descending)");
        break;
    case 's':
        sortby = sort_ssid;
        WriteStatus("Sorting by SSID");
        break;
    case 'S':
        sortby = sort_ssid_dec;
        WriteStatus("Sorting by SSID (descending)");
        break;
    case 'w':
    case 'W':
        sortby = sort_wep;
        WriteStatus("Sorting by WEP");
        break;
    case 'p':
        sortby = sort_packets;
        WriteStatus("Sorting by packet counts.");
        break;
    case 'P':
        sortby = sort_packets_dec;
        WriteStatus("Sorting by packet counts (descending)");
        break;
    case 'q':
        sortby = sort_quality;
        WriteStatus("Sorting by signal quality");
        break;
    case 'Q':
        sortby = sort_signal;
        WriteStatus("Sorting by signal strength");
        break;
    case 'x':
    case 'X':
        break;
    default:
        beep();
        return 1;
        break;
    }

    // We don't have anything that doesn't kill the window for the key event
    return 0;
}

int PanelFront::PackInput(void *in_window, int in_chr) {
    kis_window *kwin = (kis_window *) in_window;
    char sendbuf[1024];

    switch(in_chr) {
    case 'h':
    case 'H':
        SpawnHelp(KismetHelpPack);
        break;

    case 'p':
    case 'P':
        if (kwin->paused)
            kwin->paused = 0;
        else
            kwin->paused = 1;
        break;
    case 'x':
    case 'X':
    case 'q':
    case 'Q':
        snprintf(sendbuf, 1024, "!%u nopacktypes\n", (unsigned int) time(0));
        client->Send(sendbuf);
        return 0;
        break;
    default:
        break;
    }

    return 1;

}

int PanelFront::DumpInput(void *in_window, int in_chr) {
    kis_window *kwin = (kis_window *) in_window;
    char sendbuf[1024];

    switch(in_chr) {
    case 'm':
    case 'M':
        MuteToggle();
        break;
    case 'p':
    case 'P':
        // Ignore if we're pending a clear
        if (clear_dump == 1)
            break;

        if (kwin->paused)
            kwin->paused = 0;
        else
            kwin->paused = 1;
        break;

    case 'c':
    case 'C':
        if (!kwin->paused)
            clear_dump = 1;
        break;
    case 'h':
    case 'H':
        SpawnHelp(KismetHelpDump);
        break;
    case 'x':
    case 'X':
    case 'q':
    case 'Q':
        snprintf(sendbuf, 1024, "!%u nostrings \n", (unsigned int) time(0));
        client->Send(sendbuf);
        return 0;
        break;
    default:
        break;
    }

    return 1;
}

// We don't do anything special here except spawn a help and pass it on to the
// text input handler.  Details is just a slightly special text window.
int PanelFront::DetailsInput(void *in_window, int in_chr) {
    int ret;
    switch (in_chr) {
    case 'h':
    case 'H':
        SpawnHelp(KismetHelpDetails);
        break;
    case 'n':
        // Nasty hack but it works
        ret = (this->*net_win->input)(net_win, KEY_DOWN);
        details_network = last_displayed[net_win->selected];
        return ret;
        break;
    case 'p':
        ret = (this->*net_win->input)(net_win, KEY_UP);
        details_network = last_displayed[net_win->selected];
        return ret;
        break;
    default:
        return TextInput(in_window, in_chr);
        break;
    }

    return 1;
}

int PanelFront::PowerInput(void *in_window, int in_chr) {
    switch (in_chr) {
    case 'h':
    case 'H':
        SpawnHelp(KismetHelpPower);
        break;
    case 'x':
    case 'X':
    case 'q':
    case 'Q':
        return 0;
        break;
    }

    return 1;
}

int PanelFront::GpsInput(void *in_window, int in_chr) {
    kis_window *kwin = (kis_window *) in_window;

    switch (in_chr) {
    case 's':
    case 'S':
        kwin->selected = 1;
        break;
    case 'c':
    case 'C':
        kwin->selected = 0;
        break;
    case 'h':
    case 'H':
        SpawnHelp(KismetHelpGps);
        break;
    case 'x':
    case 'X':
    case 'q':
    case 'Q':
        return 0;
        break;
    }

    return 1;
}

int PanelFront::RateInput(void *in_window, int in_chr) {
    switch (in_chr) {
    case 'h':
    case 'H':
        SpawnHelp(KismetHelpRate);
        break;
    case 'x':
    case 'X':
    case 'q':
    case 'Q':
        return 0;
        break;
    }

    return 1;
}

int PanelFront::StatsInput(void *in_window, int in_chr) {
    switch (in_chr) {
    case 'h':
    case 'H':
        SpawnHelp(KismetHelpStats);
        break;
    case 'x':
    case 'X':
    case 'q':
    case 'Q':
        return 0;
        break;
    }

    return 1;
}

int PanelFront::TextInput(void *in_window, int in_chr) {
    kis_window *kwin = (kis_window *) in_window;

    switch (in_chr) {
    case KEY_UP:
    case '-':
        if (kwin->start != 0) {
            kwin->start--;
        }
        break;
    case KEY_DOWN:
    case '+':
        if (kwin->end < (int) kwin->text.size() - 1 && kwin->end != 0) {
            kwin->start++;
        }
        break;
    case 'x':
    case 'X':
    case 'q':
    case 'Q':
        return 0;
        break;
    default:
        return 1;
        break;
    }

    return 1;
}

void PanelFront::MuteToggle() {
    if (muted) {
        speech = old_speech;
        sound = old_sound;
        muted = 0;
        WriteStatus("Restoring sound");
    } else if (sound != 0 || speech != 0) {
        old_speech = speech;
        old_sound = sound;
        sound = 0;
        speech = 0;
        muted = 1;
        WriteStatus("Muting sound");
    } else if (sound == 0 && speech == 0) {
        WriteStatus("Sound not enabled.");
    }
}

int PanelFront::AlertInput(void *in_window, int in_chr) {
    switch (in_chr) {
    case 'h':
    case 'H':
        //SpawnHelp(KismetHelpDetails);
        break;
    default:
        return TextInput(in_window, in_chr);
        break;
    }

    return 1;
}


#endif