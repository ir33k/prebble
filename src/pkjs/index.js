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
        "type": "select",
        "messageKey": "BGTYPE",
        "label": "Type",
        "defaultValue": 1,
        "options": [
          { "value": 0, "label": "None" },
          { "value": 1, "label": "Solid color" },
          { "value": 2, "label": "Battery charge" }
        ]
      },
      {
        "id": "bgtype-none-text",
        "type": "text",
        "defaultValue": "Background disabled. It will be white."
      },
      {
        "type": "color",
        "messageKey": "BGCOLOR",
        "label": "Color",
        "allowGray": true
      },
      {
        "id": "bgtype-battery-text",
        "type": "text",
        "defaultValue": "Background color will reflect battery charge level."
      },
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
          { "value": 2, "label": "Dots" }
        ]
      },
      {
        "type": "color",
        "messageKey": "FGCOLOR",
        "label": "Color",
        "allowGray": true
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
        "messageKey": "VIBEBT",
        "label": "Bluetooth connection",
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
          { "value": "%a %m.%d", "label": "Su 11.18" },
          { "value": "%B %d",    "label": "November 18" },
          { "value": "%b %d",    "label": "Nov 18" },
          { "value": "%A %d",    "label": "Sunday 18" },
          { "value": "%a %d",    "label": "Su 18" },
          { "value": "%Y.%m.%d", "label": "2022.11.18" },
          { "value": "%d.%m.%Y", "label": "18.11.2022" },
          { "value": "%m.%d.%Y", "label": "11.18.2022" }
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
      this.getItemByMessageKey('BGTYPE'),
      this.getItemById('bgtype-none-text'),
      ["1", "2"]
    )
    hideOnValue(
      this.getItemByMessageKey('BGTYPE'),
      this.getItemByMessageKey('BGCOLOR'),
      ["0", "2"]
    )
    hideOnValue(
      this.getItemByMessageKey('BGTYPE'),
      this.getItemById('bgtype-battery-text'),
      ["0", "1"]
    )
    hideOnValue(
      this.getItemByMessageKey('FGTYPE'),
      this.getItemByMessageKey('FGCOLOR'),
      ["0"]
    )
    hideOnValue(
      this.getItemByMessageKey('FGTYPE'),
      this.getItemByMessageKey('FGBT'),
      ["0"]
    )
  }.bind(this))
})
