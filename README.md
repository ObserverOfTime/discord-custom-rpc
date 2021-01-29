# discord-custom-rpc

Custom Discord Rich Presence for Linux written in C.

Depends on `json-c` & `uuid` and is configured during compilation.

First, create an [application][] and add some Rich Presence assets.

[application]: https://discord.com/developers/applications/

Then, create an `Env.mak` file to store the configuration.

```make
# your client ID, required
CLIENT_ID =
# rich presence details, optional
DETAILS =
# rich presence state, optional
STATE =
# large image key, optional
LARGE_IMAGE =
# large image text, optional
LARGE_TEXT =
# small image key, optional
SMALL_IMAGE =
# small image text, optional
SMALL_TEXT =
# set to any value to enable debugging
DEBUG =
```

Now, you can compile and run the application with `make`.

Alternatively, use `make rpc` to only compile it.
<br/>You can run the application later with `./rpc`
