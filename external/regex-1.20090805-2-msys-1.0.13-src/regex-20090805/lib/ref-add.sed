/^# Packages using this file: / {
  s/# Packages using this file://
  ta
  :a
  s/ regex / regex /
  tb
  s/ $/ regex /
  :b
  s/^/# Packages using this file:/
}
