var Clay = require('pebble-clay')

var vibs = [
  { "value": 0, "label": "None" },
  { "value": 1, "label": "Short" },
  { "value": 2, "label": "Long" },
  { "value": 3, "label": "Double" }
]

new Clay([
  {
    "type": "heading",
    "defaultValue": "pRebble"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Background"
      },
      {
        "type": "color",
        "messageKey": "BGCOLOR",
        "label": "Color",
        "allowGray": true
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Background pattern"
      },
      {
        "type": "select",
        "messageKey": "FGTYPE",
        "label": "Type",
        "defaultValue": 1,
        "options": [
          { "value": 0, "label": "None" },
          { "value": 1, "label": "Diagonal lines" },
          { "value": 2, "label": "Dots" },
          { "value": 3, "label": "Dithering" },
          { "value": 4, "label": "Battery charge" }
        ]
      },
      {
        "id": "fgtype-battery-text",
        "type": "text",
        "defaultValue": "Dithering will reflect battery charge percent.  0% of battery equals background color, 100% equals pattern color.  Except if you are using black then dithering will avoid getting pure black."
      },
      {
        "type": "color",
        "messageKey": "FGCOLOR",
        "label": "Color"
      },
      {
        "type": "slider",
        "messageKey": "FGDITHER",
        "defaultValue": 144,
        "label": "Dithering percent",
        "description": "Density of dithering pattern.",
        "min": 0,
        "max": 252,
        "step": 4
      },
      {
        "type": "toggle",
        "messageKey": "FGBT",
        "label": "Hide on Bluetooth disconnect",
        "defaultValue": true
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Vibrations"
      },
      {
        "type": "select",
        "messageKey": "VIBEBT0",
        "label": "Bluetooth disconnected",
        "defaultValue": 0,
        "options": vibs
      },
      {
        "type": "select",
        "messageKey": "VIBEBT1",
        "label": "Bluetooth connected",
        "defaultValue": 0,
        "options": vibs
      },
      {
        "type": "select",
        "messageKey": "VIBEH",
        "label": "Hourly",
        "defaultValue": 0,
        "options": vibs
      },
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Other"
      },
      {
        "type": "select",
        "messageKey": "DATE",
        "label": "Date format",
        "defaultValue": 1,
        "options": [
          { "value": "",         "label": "Default" },
          { "value": "%A %m.%d", "label": "Sunday 11.18" },
          { "value": "%a %m.%d", "label": "Sun 11.18" },
          { "value": "%B %d",    "label": "November 18" },
          { "value": "%b %d",    "label": "Nov 18" },
          { "value": "%A %d",    "label": "Sunday 18" },
          { "value": "%a %d",    "label": "Sun 18" },
          { "value": "%Y.%m.%d", "label": "2022.11.18" },
          { "value": "%d.%m.%Y", "label": "18.11.2022" },
          { "value": "%m.%d.%Y", "label": "11.18.2022" },
          { "value": "%a %b %d", "label": "Sun Nov 18" }
        ]
      },
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save"
  }
], function () {
  // Call CB function right away and on event NAME of EL item.
  function on(el, name, cb) {
    cb()
    el.on(name, cb)
  }
  // Hide DEST item when SRC item value is one of VALUES.
  function hideOnValue(src, dest, values) {
    on(src, "change", function () {
      dest[values.indexOf(src.get()) === -1 ? "show" : "hide"]()
    })
  }

  this.on(this.EVENTS.AFTER_BUILD, function () {
    hideOnValue(
      this.getItemByMessageKey('FGTYPE'),
      this.getItemById('fgtype-battery-text'),
      ["0", "1", "2", "3"]
    )
    hideOnValue(
      this.getItemByMessageKey('FGTYPE'),
      this.getItemByMessageKey('FGCOLOR'),
      ["0"]
    )
    hideOnValue(
      this.getItemByMessageKey('FGTYPE'),
      this.getItemByMessageKey('FGDITHER'),
      ["0", "1", "2", "4"]
    )
    hideOnValue(
      this.getItemByMessageKey('FGTYPE'),
      this.getItemByMessageKey('FGBT'),
      ["0"]
    )
  }.bind(this))
})
