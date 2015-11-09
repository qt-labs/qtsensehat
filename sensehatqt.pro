load(configure)
qtCompileTest(rtimulib)

load(qt_parts)

!config_rtimulib {
    warning("RTIMULib not found. Make sure the sense-hat and librtimulib-dev packages are installed.")
    SUBDIRS =
}
