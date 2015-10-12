#include <libevdev-1.0/libevdev/libevdev.h>
#include <mpd/client.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>

static bool check_error(struct mpd_connection *client)
{
  if (mpd_connection_get_error(client) != MPD_ERROR_SUCCESS) {
    printf("Error: %s (Retry in 1 second...)\n",
           mpd_connection_get_error_message(client));
    if (!mpd_connection_clear_error(client)) {
      mpd_connection_free(client);
      sleep(1);
      return true;
    }
  }
  return (client == NULL);
}

static struct mpd_connection *connect(const char *host, unsigned port)
{
  struct mpd_connection *client = NULL;
  for (;;) {
    client = mpd_connection_new(host, port, 2000);
    if (check_error(client)) {
      continue;
    }

    break;
  }

  return client;
}

int main(int argc, char const *argv[])
{
  if (argc < 5) {
    printf("Usage: evmpd HOSTNAME PORT DEVICE HALT_COMMAND\n");
    return EXIT_FAILURE;
  }

  int previous_sond_id = -1;
  enum mpd_state prev_state = MPD_STATE_UNKNOWN;
  struct mpd_connection *client = connect(argv[1], strtol(argv[2], NULL, 10));

  struct libevdev *dev = NULL;
  int fd;
  int rc = 1;

  fd = open(argv[3], O_RDONLY | O_NONBLOCK);
  rc = libevdev_new_from_fd(fd, &dev);
  if (rc < 0) {
    fprintf(stderr, "Failed to init libevdev (%d)\n", strerror(-rc));
    exit(1);
  }
  printf("Input device name: \"%s\"\n", libevdev_get_name(dev));
  printf("Input device ID: bus %#x vendor %#x product %#x\n",
         libevdev_get_id_bustype(dev),
         libevdev_get_id_vendor(dev), libevdev_get_id_product(dev));

  do {
    struct input_event ev;

    rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);

    if (rc == 0 && ev.value == 0) {
      if (libevdev_event_is_code(&ev, EV_KEY, KEY_F4)) {
        system(argv[4]);
        printf("shutdown\n");

      } else if (libevdev_event_is_code(&ev, EV_KEY, KEY_PLAYPAUSE)) {
        mpd_run_toggle_pause(client);
        printf("toggle-pause\n");

      } else if (libevdev_event_is_code(&ev, EV_KEY, KEY_STOPCD)) {
        mpd_run_stop(client);
        printf("stop\n");

      } else if (libevdev_event_is_code(&ev, EV_KEY, KEY_PREVIOUSSONG)) {
        mpd_run_previous(client);
        printf("previous\n");

      } else if (libevdev_event_is_code(&ev, EV_KEY, KEY_NEXTSONG)) {
        mpd_run_next(client);
        printf("next\n");

      } else if (libevdev_event_is_code(&ev, EV_KEY, KEY_VOLUMEUP)) {
        struct mpd_status *status = mpd_run_status(client);
        int volume = mpd_status_get_volume(status);

        volume += 5;
        if (volume > 100) {
          volume = 100;
        }

        mpd_run_set_volume(client, volume);
        printf("set-volume %d\n", volume);

      } else if (libevdev_event_is_code(&ev, EV_KEY, KEY_VOLUMEDOWN)) {
        struct mpd_status *status = mpd_run_status(client);
        int volume = mpd_status_get_volume(status);

        volume -= 5;
        if (volume < 0) {
          volume = 0;
        }

        mpd_run_set_volume(client, volume);
        printf("set-volume %d\n", volume);
        /* } else { */
        /*   printf("Event: %s %s %d\n", */
        /*          libevdev_event_type_get_name(ev.type), */
        /*          libevdev_event_code_get_name(ev.type, ev.code), */
        /*          ev.value); */
      }
    }

  }
  while (rc == 1 || rc == 0 || rc == -EAGAIN);

  if (client != NULL) {
    mpd_connection_free(client);
  }
  return EXIT_SUCCESS;
}
