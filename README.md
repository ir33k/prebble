pRebble - Pebble REBBLE watchface
=================================

Pebble watchface created during [Rebble][] [Hackathon #001][].
Inspired by Rebble official logo and Pebble visual language (UI design
and animations).

## Appstore links

- [pRebble - aplite][]
- [pRebble - basalt][]
- [pRebble - chalk][]
- [pRebble - diorite][]

## Features

- Digital and analog time.
- Support both 24h/12h digital time format.
- Date (week name, month and day).
- Compatible with Quick View.
- Quick animation after each minute.
- Support works on all pebbles, special care for Pebble Round.

## Settings

- Custom background color.
- Background color can show battery status, green up to 60%, blue up
  to 30% and red below that. On black and white screens it's gray up
  to 30%, then white.
- Custom background patterns. Only 2 for now.
- Custom pattern color.
- Hide pattern color on Bluetooth disconnect.
- Vibrations on Bluetooth connection change.
- Vibrations on each hour.
- Custom date formats.

## Screenshots

![aplite](src/img/aplite.gif)
![basalt](src/img/basalt.gif)
![chalk](src/img/chalk01.png)

## Recordings of animations

![Green version showing animation](devlog/animation03.gif)
![Quick View example](devlog/animation04.gif)

## Icons

![25bw](icon/25bw.png)
![25orange](icon/25orange.png)
![25](icon/25.png)
![25red](icon/25red.png)
![50red](icon/50red.png)
![80red](icon/80red.png)

## Rebble emoji for Rebble Discord server

![emoji128](icon/emoji128.png)
![emoji32](icon/emoji32.png)

## Development log

### 2022.11.06 Sun 18:18 - Installing SDK, first try

Rebble prepared convenient "Developer VM" but I would very much prefer
to have more lightweight setup.  That means installing SDK locally.
On my Debian 11 with no signs of Python2, which is required, it might
be a bit more difficult.  Let's try anyway.

I followed official [Pebble SDK installation instruction][].  But
before that I noticed warning message that Python 2.7 and pip2 are
required.  So I installed few packages to get proper Python versions
and then I followed article called [Install pip for Python2.7 in
Debian 11 Bullseye][].  Strange coincidence that I found help on Emacs
user blog and I'm Emacs user too.  Thanks bro!  Works like a charm.
But I have to remember to use `python2 -m pip` instead of `pip`.

	$ sudo apt install python2
	$ sudo apt install python2.7
	$ sudo apt install python-dev
	$ wget https://bootstrap.pypa.io/pip/2.7/get-pip.py
	$ python2 get-pip.py
	$ python2 -m pip --version
	$ python2 -m pip help

Now following the [Pebble SDK installation instruction][] I could do
everything with few commends modified to fit my pip situation.

	$ cd ~/pebble/pebble-sdk-4.5-linux64
	$ python2 -m pip install virtualenv
	$ python2 -m virtualenv .env
	$ source .env/bin/activate
	$ python2 -m pip install -r requirements.txt
	$ deactivate
	$ sudo apt install libsdl1.2debian libfdt1 libpixman-1-0

I'm good to go.  I will do the [Pebble watchface tutorial part 1][] to
test my local SDK installation.

### 2022.11.06 Sun 18:48 - Tutorial part 1, fail quick

	$ pebble new-project prebble
	Pebble collects metrics on your usage of our developer tools.
	We use this information to help prioritise further development of our tooling.

	If you cannot respond interactively, create a file called ENABLE_ANALYTICS or
	NO_TRACKING in '/home/irek/.pebble-sdk/'.

	Would you like to opt in to this collection? [y/n] n
	Traceback (most recent call last):
	  File "/home/irek/pebble/pebble-sdk-4.5-linux64/pebble-tool/pebble.py", line 7, in <module>
		pebble_tool.run_tool()
	  File "/home/irek/pebble/pebble-sdk-4.5-linux64/pebble-tool/pebble_tool/__init__.py", line 32, in run_tool
		analytics_prompt()
	  File "/home/irek/pebble/pebble-sdk-4.5-linux64/pebble-tool/pebble_tool/util/analytics.py", line 242, in analytics_prompt
		post_event("sdk_analytics_opt_out", force=True)
	  File "/home/irek/pebble/pebble-sdk-4.5-linux64/pebble-tool/pebble_tool/util/analytics.py", line 213, in post_event
		PebbleAnalytics.get_shared().submit_event(event, **data)
	  File "/home/irek/pebble/pebble-sdk-4.5-linux64/pebble-tool/pebble_tool/util/analytics.py", line 109, in submit_event
		requests.post(self.TD_SERVER, data=fields)
	  File "/home/irek/pebble/pebble-sdk-4.5-linux64/.env/lib/python2.7/site-packages/requests/api.py", line 109, in post
		return request('post', url, data=data, json=json, **kwargs)
	  File "/home/irek/pebble/pebble-sdk-4.5-linux64/.env/lib/python2.7/site-packages/requests/api.py", line 50, in request
		response = session.request(method=method, url=url, **kwargs)
	  File "/home/irek/pebble/pebble-sdk-4.5-linux64/.env/lib/python2.7/site-packages/requests/sessions.py", line 465, in request
		resp = self.send(prep, **send_kwargs)
	  File "/home/irek/pebble/pebble-sdk-4.5-linux64/.env/lib/python2.7/site-packages/requests/sessions.py", line 573, in send
		r = adapter.send(request, **kwargs)
	  File "/home/irek/pebble/pebble-sdk-4.5-linux64/.env/lib/python2.7/site-packages/requests/adapters.py", line 415, in send
		raise ConnectionError(err, request=request)
	requests.exceptions.ConnectionError: ('Connection aborted.', gaierror(-2, 'Name or service not known'))

Not the best start.  Let's see what went wrong.

Ha!  The comments you find in source code ^^

	# ~/pebble/pebble-sdk-4.5-linux64/pebble-tool/pebble_tool/__init__.py:
	def run_tool(args=None):
		urllib3.disable_warnings()  # sigh. :(
		logging.basicConfig()

I think that value of `DOWNLOAD_SERVER` variable might be wrong.

	# ~/pebble/pebble-sdk-4.5-linux64/pebble-tool/pebble_tool/sdk/manager.py
	class SDKManager(object):
		DOWNLOAD_SERVER = "https://sdk.getpebble.com"

This is probably outdated and should be changed to valid Rebble URL.
I'm starting to think that there is SDK version already adjusted by
Rebble team.  Let's look for that.

Yea...  There is this official [Rebble SDK instruction][].  Let's
quickly install SDK again from proper source.

### 2022.11.06 Sun 19:12 - First try ^-^

Ok, now it should be good.  Rebble version of SDK installed.  Path
updated.  Python dependencies installed, again.  And I see that there
is no need to do tutorial in order to verify SDK installation.  There
is a better way.  All described in Rebble SDK installation manual.
Props for that.

	$ pebble sdk install latest
	$ mkdir ~/pebble/tmp
	$ cd ~/pebble/tmp
	$ git clone https://github.com/pebble-examples/cards-example
	$ cd cards-example
	$ pebble build
	$ pebble install --emulator basalt

Builds fast, emulation works.  I'm ready!  It wasn't that hard.

### 2022.11.06 Sun 23:08 - Another great comment from RebbleOS source

	// rebble-os/rwatch/graphics/font_loader.c:53

	// get a system font and then cache it. Ugh.
	// TODO make this not suck (RAM)
	GFont fonts_get_system_font(const char *font_key)

### 2022.11.18 Fri 12:43 - Hackathon first day

Hackathon starts at 19:00.  Let's prepare a bit before that.  Last
week I went through whole tutorial.  I also watched archive of
[Presentation Designing Apps for Pebble][] and another interesting
[Presentation Developing for Round][].  What I can do now is to
prepare repository and basic project structure that allows to run
empty watchface in emulator.  I should also be able to run watchface
on actual watch.  I haven't done that before.  I can also read more
about vector graphics.  Tutorial shows only how to deal with bitmaps.

### 2022.11.18 Fri 14:37 - Rebble icon and Discord emoji

I couldn't resist to start doing something already.  So as warm-up I
did icon of pRebble watch.  It's pixel perfect in smallest Pebble icon
size that is 25x25 px as far as i know.  I posted it on Discord Rebble
#hackathon channel.  Soon after @Will0 replied:

> Looks great! I've added it as a server emoji: :rebblewatch:

WoW!  That's so great ^-^ I followed up with slightly bigger icon
aligned with Discord emoji resolution specification.

I used orange color for icons background.  It's the closest orange
from Pebble color pallet that match orange used in Rebble logo.  But I
think that main weatchface color should be red.  Rebellion is often
associated with red color.  But I will decide about that later.

### 2022.11.19 Sat 08:18 - Day one

Yesterday I focused mostly on design.  File [design.svg](./design.svg)
was created and besides few chosen graphics on main page there are
many abandoned ideas outside of the page.

Initially I gravitated towards big very abstract analog watchface.
Then I created simple icon style analog watchface that both in layout
and style should resemble regular Pebble icons that appears when you
toggle quiet time, mute app etc.  The last idea was to play with light
and shadows using big shapes.  I made silhouette of original Pebble
using just one big black shape and few light spots with white screen.
This watchface was fully digital.  Initially I rly liked it but
usually picture of Pebble watch inside Pebble watch is not a good idea
and it was the same in this case.

So I went back to second concept.  After refining it and testing on
actual watch I realized that this is the on that meets my goals and
looks very clean on watch.  My new favorite watchface!

![watchface design - black-white](devlog/wf01.png)
![watchface design - green](devlog/wf02.png)
![watchface design - red](devlog/wf03.png)

I mentioned testing designs on watch itself.  For that I created new
Pebble project.  With knowledge from tutorial I was able to quickly
make it display single bitmap.  This was used to test all designs.

### 2022.11.19 Sat 08:28 - What next?

> After three days without programming, life becomes meaningless.

Enough of design.  Time to code.  I will start by creating red
background with subtle stripes across whole screen.  It would be great
to have option to customize background to your liking.  But that is
not necessary right now.  So I will hardcode default background first.

Then I can add white background for digital time and time itself.
After that icon.  There will be a lot of fun with vector graphic API.
I did some reading yesterday.  Making static image should not be a big
deal but animation might take some time.

### 2022.11.19 Sat 09:46 - What is your background sir?

Background is ready.  Pure red with black lines across the screen.
Note the details.  Lines are positioned in a way that makes them start
and end in the corners.  This characteristic defines gap between lines
because it has to be precise to make that happen.

![screenshot01 - background with pattern](devlog/screenshot01.png)

### 2022.11.19 Sat 11:03 - The time has come to load fonts

I have time and date.  It was easy because there is nothing fancy
about it.  I used fonts from [Pebble system fonts][] lib.  I planned
that from beginning.  This should bring me even closer to have look
and feel of original Pebble UI.

![screenshot02 - time and date](devlog/screenshot02.png)

Now for the main event.  Drawing vector graphics.  I will make a break
tho.  I have breakfast on my mind.  I shouldn't stop til nightfall
gentleman, but second breakfast will do fine 🍎.

### 2022.11.19 Sat 17:34 - First Pebble Draw Commands (PDC)

Ok so I have SVG file.  Now I need to convert it to `.pdc` with
`svg2pdc.py`, but running this program results in error.

	$ python2 /home/irek/pebble/cards-example/tools/svg2pdc.py analog.svg 
	Traceback (most recent call last):
	  File "/home/irek/pebble/cards-example/tools/svg2pdc.py", line 17, in <module>
	    import svg.path
	ImportError: No module named svg.path

Looks like `svg` module is missing.  I think I can install it with:

	$ python2 -m pip install svg.path
	
So that worked but now I have different error.

	$ python2 /home/irek/pebble/cards-example/tools/svg2pdc.py analog.svg
	Traceback (most recent call last):
	  File "/home/irek/pebble/cards-example/tools/svg2pdc.py", line 17, in <module>
	    import svg.path
	  File "/home/irek/.local/lib/python2.7/site-packages/svg/path/__init__.py", line 1, in <module>
	    from .path import Path, Move, Line, Arc, Close  # noqa: 401
	  File "/home/irek/.local/lib/python2.7/site-packages/svg/path/path.py", line 105
	    return f"Line(start={self.start}, end={self.end})"
	                                                     ^
	SyntaxError: invalid syntax

Syntax error in module file?  This looks almost like if installed
module is not written for python2.

And probably yes.  Because in module [svg.path CHANGES.txt][] file I
can clearly see that they dropped support for Python 2 in first half
of this year.  So I think that I need to get older version manually.
Let's try to do that.  I will try to get version 4.1.

	$ git clone https://github.com/regebro/svg.path.git
	$ cd svg.path
	$ git checkout 4.1
	$ rm -rf ~/.local/lib/python2.7/site-packages/svg
	$ cp -r src/svg ~/.local/lib/python2.7/site-packages/
	
FIRST TRY!  It works.  I mean the script.  I still have errors but
different kind of errors.  Looks like my path is not pixel perfect.
After quick fix I got another error.

	$ python2 /home/irek/pebble/cards-example/tools/svg2pdc.py analog.svg
	Traceback (most recent call last):
	  File "/home/irek/pebble/cards-example/tools/svg2pdc.py", line 646, in <module>
	    main(args)
	  File "/home/irek/pebble/cards-example/tools/svg2pdc.py", line 621, in main
	    args.precise)
	  File "/home/irek/pebble/cards-example/tools/svg2pdc.py", line 589, in create_pdc_from_path
	    size, commands, error = parse_svg_image(path, precise, raise_error)
	  File "/home/irek/pebble/cards-example/tools/svg2pdc.py", line 549, in parse_svg_image
	    cmd_list, error = get_commands(translate, root, precise, raise_error)
	  File "/home/irek/pebble/cards-example/tools/svg2pdc.py", line 463, in get_commands
	    cmd_list, err = get_commands(translate, child, precise, raise_error, truncate_color)
	  File "/home/irek/pebble/cards-example/tools/svg2pdc.py", line 459, in get_commands
	    translate_strs = re.search(r'(?:translate\()(.*),(.*)\)',transform).group(1,2)
	AttributeError: 'NoneType' object has no attribute 'group'

That was due to my mistake of grouping few elements together.  Script
don't support SVG groups and it was mention in documentation so let's
try again without any groups.

I got my `.pdc` file, despite few "Invalid point" messages, because
Inkscape seems to be very inaccurate even with snap to grid option.
Let's try to draw that in watchface.

### 2022.11.19 Sat 18:12 - It's alive!

It works!  That's great.  Now I need to adjust position.

### 2022.11.19 Sat 18:28

It's great.  But I still see some jagged edges.  Inkscape you have
failed me for the last time.

### 2022.11.19 Sat 19:07

This is insane.  Inkscape rly like to force you to use floats.  And it
looks like it remembers which point was moved and it will not update
it position in output file unless you move that specific point.  So
after manually moving each point back and forth to snap it to pixel
grid i finally have expected result.  It's perfect!  But I consider
switching to different tool.  Maybe Figma?

![screenshot03 - aplite with static analog](devlog/screenshot03.png)
![screenshot04 - basalt with static analog](devlog/screenshot04.png)

### 2022.11.19 Sat 23:31 - Hands up!

I have analog clock hands.  At the beginning code resulted in not very
straight lines.  It took me a while to realize that it's impossible to
calculate point right above middle of the circle if there is no center
pixel.  Code have chose one on the right or one on the left.  But
adjusting layer size to make it have middle pixel solved this problem.

![screenshot05 - aplite with working analog](devlog/screenshot05.png)
![screenshot06 - basalt with working analog](devlog/screenshot06.png)

Now what?

### 2022.11.20 Sun 00:56 - You spine me round round

After just few adjustments watchface works on Pebble Round.  I added
slightly rounded corners to digital time background.  After that I had
an idea to make this background an circle in Pebble Round.  This makes
sense and looks great.  Another small change was to display
abbreviation of month name on Pebble Round.  It fits better small
space at the bottom.

![screenshot07 - chalk support](devlog/screenshot07.png)

### 2022.11.20 Sun 11:16 - Animations

I just did my first animation.  It's very quick and simple but adds a
lot to watchface.

![animation01 - first animation](devlog/animation01.mp4)

### 2022.11.20 Sun 19:09 - Inkscape </3

Ok so coding sequence animations with Pebble API is easy but preparing
SVG animation in Inkscape is horrible.  I don't think that there is
single good software for making SVG animations of this kind.  I tried
few but most are about moving and modifying whole shapes, or final
animation is a bitmap image or video.

I even spend few hours trying animate whole thing with code point by
point.  I learned quite a lot about how PDC format stores data.  But
eventually I went back to Inkscape and just animated each frame by
hand on separate layer.  Exporting was also problematic but anyway, in
the end I have my sequence animation for analog clock.  It runs on
each minute.

![animation02 - sequence analog clock animation](devlog/animation02.mp4)
![animation03 - green background](devlog/animation03.mp4)

### 2022.11.20 Sun 21:43 - Unobstructed area AKA Quick View

That was the last thing that I wanted to handle - unobstructed area.
It works just by hiding digital area section.  It looks very nice tho.
Anyway.  I wanted to add customization for background color but I
don't think there is enough time for that.  Will see.  I will prepare
everything for release and then decide.

![animation04 - quick view](devlog/animation04.mp4)

### 2022.11.20 Sun 22:13 - Publish

Releases in [Rebble Developer Portal][].  I'm done with v1.0.  Time
for rest but development is not over.  I still want to deliver
settings page with customization options.

Appstore pages for each platform:

- [pRebble - aplite][]
- [pRebble - basalt][]
- [pRebble - chalk][]
- [pRebble - diorite][]

I will add more details later.

### 2022.11.21 Mon 22:39 - After release and plans for future

Ok so everything went smoothly except that today I found bug.  Hour
analog clock hand was a bit off.  For example when time was 13:50
clock hour hand should already point more at 14:00 than 13:00 but it
wasn't.  It was a small silly mistake in code.  Fixed quickly and
watchface was updated with version 1.1.

Right after that I added watchface menu icon.  Finally!  I had it from
the beginning but since I didn't know how to add it i skipped it for
version 1.0 because I thought this will be more involve.  It was
actually pretty easy.  I actually learned how to do it from other
awesome project, [Watchface Pixel by phoeniix][].  He had separate
commit [Add menu item commit][] for that.

![pRebble menu icon](devlog/pic03.jpeg)

Then I acknowledge problem with analog clock minute hand.  It was
overlapping with analog clock arrows at 2 specific points and it looks
very strange.  I fixed it quickly by adding white border around minute
hand.  This was achieved by drawing white thicker line under actual
black minute hand line.

### 2022.11.22 Tue 04:13 - So now what?

Topics on Rebble Discord channel related to Hackathon are archived.  I
will continue my work on this watchface.  I rly want to add few things
that I planned from the start and some more.

But before that I'm doing some cleaning.  Starting with [design.svg][]
file.  Whole design process is now visible on the page.  Previously
only chosen designs where visible and everything else was outsize the
page.  Now it's easier to look at all that mess.  I also defined tags
in this repository so it's easy to jump to specific version.  This
file was cleaned up as well.  Top section is now more presentable and
it doesn't make first impression of project being WIP.

And the plan for version 2.0 is to add settings page using
[pebble-clay][] with features:

- Custom background color.
- Possibility to have background color that reflects battery level.
  On color screen from green, to orange and red.  On black and white
  from white to gray to black.
- Custom patter.  I want to make patter optional.  And when enabled
  then you can chose one of patterns.  For now we have only black
  lines.  But I already experimented with dots in [design.svg][].  And
  I have few other ideas.  Color for patter can also by custom.
- Show Bluetooth connection by hiding patter.  So when patter is
  defined it can be hidden when connection with phone is lost.
- Vibrate when Bluetooth connection change.
- Vibrate on each hour.
- Custom date format.

I like how with background I can squeeze extra features and keep
watchface as clean as it is now.  I would rly like to add weather as
well.  But I have no idea how to do that yet.  Some watchfaces shows
extra stuff on shake.  I don't like that.  So I don't know how to do
it yet.

### 2022.11.22 Tue 04:46 - Appstore stars

Watchface already has 12 stars in Appstore O_O.  This is super
awesome!  I'm happy that people like it as I rly tried to not only
meet design goals but also make it useful for every day usage.  I'm
using it since first day and it works for me very well.  It rly need
those customization tho.  It brings watchface to another level when
you can make it your own.

### 2022.11.22 Tue 20:31 - Hello sequence my old friend

	$ python2 /home/irek/pebble/cards-example/tools/svg2pdc.py -c 1 -d 12 -o resources/seq.pdc -s src/img/seq
	
### 2022.11.23 Wed 22:38 - Ideas

Change background color, pattern type and pattern color randomly on
each hour or each shake.

### 2022.11.24 Thu 03:58 - Settings

It took a while, actually the whole night of programming.  But I did
it.  I have settings panel with everything that I listed earlier.
Yes, everything!  Now for the release of v2.0.  I need to prepare new
screenshots and gif.  I don't have a strength for that tho.  So I just
released new version with change log new description and that's it for
today.  I'm not expecting to do much more.  Probably 2 or 3 new
background patterns, new screenshots for store and potential bug fixes
and some refactor but nothing major.  Everything seems to be working
fine and I have all the features that I wanted.

### 2022.11.27 Sun 22:52 - Polish

Already 21 likes in appstore.  This motivates me to polish watchface
even more.

For example I should expand Clay settings page so it describe some of
the options right there on settings page.  Like background that shows
battery level.  This should be explained right there along with image
showing which color corresponds to which battery charge level.

I can add more patterns.  I was thinking about making something that
works better on Pebble round.  And like text area with digital time
and date, pattern could have subtle animation.

Topic of animation reminds me of initial idea of creating custom
animation curve function.  I never got back to it.  Current slide up
animation is fine but since the beginning I wanted to have a little
bounce at the end.  Similar to what arrows do after they go around
analog clock.

I can polish the appstore watchface page itself by adding banner
image, updating gif and screenshots.

Code is already in pretty good shape but this OFC can always be
improved.

So there are many areas that can be polished.  I think I will be able
to do some of them in upcoming week.

### 2022.12.14 Wed 11:03 - v3.0 Dithering FTW

There it is.  I did a big refactor.  That include some small fixes.
For example there was bug not allowing you to go back to default date
format and white border around minutes clock hand was a bit off.

But the biggest change is introduction of Dithering.  Initially I
wanted to have it just to enhance how battery status is shown on black
and white screens with background color.  But it works so great that
it is used even on color display.  And it's a pattern, not background.
This makes more sense because with dithering as background other
patterns rendered on top could disappear in dithering pixels patterns.
And also dithering needs it's own color.  By having dithering as
pattern I can use pattern color for it and there is no conflict with
other patterns.  This works great.  All old patterns can still be used
as before.  The only breaking change I introduced is that on color
display battery status is now shown with dithering instead of multiple
predefined colors.  Great way of using dithering for battery status is
to for example set background color to red and pattern color to green.
Then when battery is full you have green background and it slowly
transition to red as we approaching to 0% of battery.  This can create
nice color transitions.  For example when blue and red are used you
will get purple on 50% of battery.

Implementation of dithering forced me to look into frame buffer.  This
allows to modify pixels directly and with that it's faster.  I used it
on lines pattern as well.

There are also improvements to Clay settings page.  It's now more
descriptive.  And you can now define different vibration when
Bluetooth is connected and different when it's disconnected.

### 2022.12.17 Sat 13:52 - v3.0 Released

I just released version 3.0 with new gifs and images in app store.
Only for aplite and basalt tho.  I don't have strengths to do it for
chalk.  Anyway I think that's it for now.  Unless there will be a bug
repport or I rly feel like to change / add something.

### 2023.04.22 Sat 11:02 - Revision

It has been 5 months since release of v1.0 and 4 since v3.0.  I went
back to the project to do small revision.  I found few small typos and
one include macro in wrong file.  Besides that everything looks solid
and I had no troubles reading and understanding the code even after
those 4 months.  This is a good sign especially considering that this
was my first and only Pebble project and at this point I don't rly
remember anything from Pebble API.

Looking at the Pebble store [pRebble - basalt][] is see that this
watchface got 46 hearts.  This is the best score on the first page
with all watchfaces making me very proud and happy.

### 2023.09.24 Sun 05:05 - Why won't you die?

Just watched [Pebble Smartwatch: From $230 Million to Zero][] and it
was surprising to see that despite being "dead" people still talk
about Pebble.  [Rebble Discord][] is still active too.  And there are
new watchfaces in Pebble store.  Looks like ideas are bulletproof.

And speaking of ideas.  Seeing that pRebble watchface has now more
than 60 hearts ([pRebble - basalt][]) I started thinking on new idea.
There is a chance that there will be second edition of Hackathon.
Should I join?  What should I do?

One idea is to extend this watchface with one last missing feature
that many watchfaces have.  That is weather forecasting.  Other idea
is to do something completely opposite this time.  So pRebble was all
about being practical and well designed with readability as number one
top priority.  What about going to polar opposite with something that
is fun, innovative and just to show off to friends.  You know like
[Perspective][] watchface.  Or maybe something that is just a pretty
picture.  I always loved watchfaces from [dezign999][].  I also like
the idea of having mini virtual world as watchface where you collect
points by wearing the watchface or by collecting steps and then you
can exchange them for more stuff that you can put into this world like
in [Steppet][].  I could also do an app this time.  I would probably
ended up making a game.

Anyway.  There is a time to think of another "bulletproof" idea.  And
when the time comes - I will be ready.

### 2023.12.17 Sun 06:31 - v3.0 Anniversary

The Hackathon #002 did not happen, at least not in this year.  But it
is an anniversary of v3.0 release.  And I'm writing because what is
surprising to me it that the watchface keeps getting likes.  71 is
where it stands now.  I guess I'm just glad that there are no bugs or
complains.  I'm sure I would know about them by now but there are no
issue on GitHub, no mails and no messages on Discord.  Code is rock
solid.  I went over the implementation couple of times.  Good thing
it's small.

I'm writing about all of this because sometimes it feels like now a
days you can't just create a software, you also have to maintain it
otherwise it will stop working very soon.  I guess it could happen to
Pebble watchfaces too if the PebbleOS would receive regular updates
that could potentially change it's behavior.  Funny thing but
apparently the dead platform makes software alive longer.


[Hackathon #001]: https://rebble.io/hackathon-001/
[Install pip for Python2.7 in Debian 11 Bullseye]: https://blog.emacsos.com/pip2-in-debian-11-bullseye.html
[Pebble Compass]: https://github.com/HBehrens/pebble-compass
[Pebble SDK installation instruction]: https://developer.rebble.io/developer.pebble.com/sdk/install/linux/index.html
[Pebble Time Design Kit Web Archive]: http://web.archive.org/web/20160415201215/http://blog.tackmobile.com/article/pebble-time-design-kit/
[Pebble watchface tutorial part 1]: https://developer.rebble.io/developer.pebble.com/tutorials/watchface-tutorial/part1/index.html
[Presentation Designing Apps for Pebble]: https://youtu.be/LuiK8ZiPXr4
[Presentation Developing for Round]: https://youtu.be/3a1V4n9HDvY
[Rebble SDK instruction]: https://help.rebble.io/sdk/
[Rebble icons]: https://github.com/piggehperson/rebble-icons
[Rebble]: http://rebble.io/
[Storytelling in pixels]: https://old.heydays.no/project/pebble/
[Watchface Analogous]: http://apps.rebble.io/en_US/application/5674eb2c1caa144be8000076?native=false&query=analogous&section=watchfaces
[Watchface DIGIANA002]: https://www.reddit.com/r/pebble/comments/7eynb0/watchface_of_the_day_pebble_2_minimalistic_mix/
[Pebble system fonts]: https://developer.rebble.io/developer.pebble.com/guides/app-resources/system-fonts/index.html
[svg.path CHANGES.txt]: https://github.com/regebro/svg.path/blob/d38002122fb50c63fa4a7e30cc834a60479c766a/CHANGES.txt#L79
[Rebble Developer Portal]: https://dev-portal.rebble.io/
[pRebble - aplite]: https://apps.rebble.io/en_US/application/637aa98bfdf3e30009f6398f?hardware=aplite
[pRebble - basalt]: https://apps.rebble.io/en_US/application/637aa98bfdf3e30009f6398f?hardware=basalt
[pRebble - chalk]: https://apps.rebble.io/en_US/application/637aa98bfdf3e30009f6398f?hardware=chalk
[pRebble - diorite]: https://apps.rebble.io/en_US/application/637aa98bfdf3e30009f6398f?hardware=diorite
[Watchface Pixel by phoeniix]: https://github.com/not-phoeniix/pixel/
[Add menu item commit]: https://github.com/not-phoeniix/pixel/commit/e80c8b0e012b7a8da5929d6819782f432a1b697e
[design.svg]: src/img/design.svg
[pebble-clay]: https://github.com/pebble/clay
[appstore settings]: https://apps.rebble.io/en_US/dev-settings?dev_settings=true
[Pebble Smartwatch: From $230 Million to Zero]: https://www.youtube.com/watch?v=SRdMQwQT47k
[Rebble Discord]: https://discordapp.com/invite/aRUAYFN
[Perspective]: https://apps.rebble.io/en_US/application/52da8234227cbe482b000120?section=watchfaces
[dezign999]: https://apps.rebble.io/en_US/developer/52f03b26a0cb6a3454000dfb/1?hardware=basalt&query=darth%2525252520sweater&section=watchfaces
[Steppet]: https://apps.rebble.io/en_US/application/57c90ebebe5ad0d0cf000041?section=watchfaces
