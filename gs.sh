curl -H "Accept:*/*" \
     -H "Accept-Language:en-US,en;q=0.9" \
     -H "Connection:keep-alive" \
     -H "Range:bytes=0-" \
     -H "Referer:https://centova57.instainternet.com/proxy/kodairagamradio?mp=/stream" \
     -H "User-Agent:Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/127.0.0.0 Safari/537.36" \
     "https://centova57.instainternet.com/proxy/kodairagamradio?mp=/stream" | \
gst-launch-1.0 fdsrc ! decodebin ! audioconvert ! audioresample ! autoaudiosink
