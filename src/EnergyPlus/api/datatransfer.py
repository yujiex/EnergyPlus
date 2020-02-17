from ctypes import cdll, c_int, c_char_p, c_void_p
from pyenergyplus.common import RealEP, EnergyPlusException
from typing import Union


class DataExchange:
    """
    This API class enables data transfer between EnergyPlus and a client.  Output variables and meters are treated as
    "sensor" data.  A client should get a handle (integer) using one of the worker methods then make calls to get data
    using that handle.  Some specific variables in EnergyPlus are controllable as actuators.  These work the same way,
    in that a client should get an integer handle to an actuator and then use the workers to set the value on the
    actuator.  There are also some static data members in EnergyPlus that are exposed as "internal" variables.  These
    variables hold static data such as zone floor area and volume, so they do not change during a simulation, but are
    constant once they are assigned.  Even with this difference, the variables are still handled the same way, by
    getting an integer handle and then accessing the data using that handle.

    This data transfer class is used in one of two workflows:

    - When a outside tool is already running EnergyPlus using the Runtime API, and data transfer is to be made during
      callback functions. In this case, the script should create a DataTransfer API class by calling the `data_transfer`
      method on the main API class, never trying to create this class directly.
    - When a Python script is used in the EnergyPlus Python Plugin System, and the user runs the EnergyPlus binary.  In
      this case, the plugin may need access to state data to make control decisions, and this class enables that.  The
      plugin base class automatically creates an instance of this class, so client plugins that inherit that class will
      have a `self.transfer` instance of this class available, and should *not* attempt to create another one.

    In the Python Plugin case, the client Python code may make use of the methods to get/set plugin global variables,
    but only in the Python Plugin cases.  For the outside tool API usage, plugin global variables are not available, and
    data should be shared in the outside calling code.
    """

    def __init__(self, api: cdll, running_as_python_plugin: bool = False):
        self.api = api
        self.running_as_python_plugin = running_as_python_plugin
        self.api.listAllAPIDataCSV.argtypes = []
        self.api.listAllAPIDataCSV.restype = c_char_p
        self.api.apiDataFullyReady.argtypes = []
        self.api.apiDataFullyReady.restype = c_int
        self.api.requestVariable.argtypes = [c_char_p, c_char_p]
        self.api.requestVariable.restype = c_void_p
        self.api.getVariableHandle.argtypes = [c_char_p, c_char_p]
        self.api.getVariableHandle.restype = c_int
        self.api.getMeterHandle.argtypes = [c_char_p]
        self.api.getMeterHandle.restype = c_int
        self.api.getActuatorHandle.argtypes = [c_char_p, c_char_p, c_char_p]
        self.api.getActuatorHandle.restype = c_int
        self.api.getVariableValue.argtypes = [c_int]
        self.api.getVariableValue.restype = RealEP
        self.api.getMeterValue.argtypes = [c_int]
        self.api.getMeterValue.restype = RealEP
        self.api.setActuatorValue.argtypes = [c_int, RealEP]
        self.api.setActuatorValue.restype = c_void_p
        self.api.resetActuator.argtypes = [c_int]
        self.api.resetActuator.restype = c_void_p
        self.api.getInternalVariableHandle.argtypes = [c_char_p, c_char_p]
        self.api.getInternalVariableHandle.restype = c_int
        self.api.getInternalVariableValue.argtypes = [c_int]
        self.api.getInternalVariableValue.restype = RealEP
        # some simulation data values are available for plugins or regular runtime calls
        self.api.year.argtypes = []
        self.api.year.restype = c_int
        self.api.month.argtypes = []
        self.api.month.restype = c_int
        self.api.dayOfMonth.argtypes = []
        self.api.dayOfMonth.restype = c_int
        self.api.dayOfWeek.argtypes = []
        self.api.dayOfWeek.restype = c_int
        self.api.dayOfYear.argtypes = []
        self.api.dayOfYear.restype = c_int
        self.api.daylightSavingsTimeIndicator.argtypes = []
        self.api.daylightSavingsTimeIndicator.restype = c_int
        self.api.hour.argtypes = []
        self.api.hour.restype = c_int
        self.api.currentTime.argtypes = []
        self.api.currentTime.restype = c_int
        self.api.minutes.argtypes = []
        self.api.minutes.restype = c_int
        self.api.holidayIndex.argtypes = []
        self.api.holidayIndex.restype = c_int
        self.api.sunIsUp.argtypes = []
        self.api.sunIsUp.restype = c_int
        self.api.isRaining.argtypes = []
        self.api.isRaining.restype = c_int
        self.api.systemTimeStep.argtypes = []
        self.api.systemTimeStep.restype = RealEP
        self.api.currentEnvironmentNum.argtypes = []
        self.api.currentEnvironmentNum.restype = c_int
        self.api.warmupFlag.argtypes = []
        self.api.warmupFlag.restype = c_int
        self.api.getPluginGlobalVariableHandle.argtypes = [c_char_p]
        self.api.getPluginGlobalVariableHandle.restype = c_int
        self.api.getPluginGlobalVariableValue.argtypes = [c_int]
        self.api.getPluginGlobalVariableValue.restype = RealEP
        self.api.setPluginGlobalVariableValue.argtypes = [c_int, RealEP]
        self.api.setPluginGlobalVariableValue.restype = c_void_p
        self.api.getPluginTrendVariableHandle.argtypes = [c_char_p]
        self.api.getPluginTrendVariableHandle.restype = c_int
        self.api.getPluginTrendVariableValue.argtypes = [c_int, c_int]
        self.api.getPluginTrendVariableValue.restype = RealEP
        self.api.getPluginTrendVariableAverage.argtypes = [c_int, c_int]
        self.api.getPluginTrendVariableAverage.restype = RealEP
        self.api.getPluginTrendVariableMin.argtypes = [c_int, c_int]
        self.api.getPluginTrendVariableMin.restype = RealEP
        self.api.getPluginTrendVariableMax.argtypes = [c_int, c_int]
        self.api.getPluginTrendVariableMax.restype = RealEP
        self.api.getPluginTrendVariableSum.argtypes = [c_int, c_int]
        self.api.getPluginTrendVariableSum.restype = RealEP
        self.api.getPluginTrendVariableDirection.argtypes = [c_int, c_int]
        self.api.getPluginTrendVariableDirection.restype = RealEP

    def list_available_api_data_csv(self) -> str:
        """
        Lists out all API data stuff in an easily parseable CSV form
        :return:
        """
        return self.api.listAllAPIDataCSV()

    def api_data_fully_ready(self) -> bool:
        success = self.api.apiDataFullyReady()
        if success == 0:
            return True
        return False

    def request_variable(self, variable_name: Union[str, bytes], variable_key: Union[str, bytes]) -> None:
        """
        Request output variables so they can be accessed during a simulation.

        In EnergyPlus, not all variables are available by default.  If they were all available, there would be a
        terrible memory impact.  Instead, only requested and necessary variables are kept in memory.  When running
        EnergyPlus as a program, including when using Python Plugins, variables are requested through input objects.
        When running EnergyPlus as a library, variables can also be requested through this function call.  This
        function has the same signature as the get_variable_handle function, which is used to then request the ID
        of a variable once the simulation has begun.

        :param variable_name: The name of the variable to retrieve, e.g. "Site Outdoor Air DryBulb Temperature", or
                              "Fan Air Mass Flow Rate"
        :param variable_key: The instance of the variable to retrieve, e.g. "Environment", or "Main System Fan"
        :return: Nothing
        """
        if isinstance(variable_name, str):
            variable_name = variable_name.encode('utf-8')
        if isinstance(variable_key, str):
            variable_key = variable_key.encode('utf-8')
        self.api.requestVariable(variable_name, variable_key)

    def get_variable_handle(self, variable_name: Union[str, bytes], variable_key: Union[str, bytes]) -> int:
        """
        Get a handle to an output variable in a running simulation.

        The arguments passed into this function do not need to be a particular case, as the EnergyPlus API
        automatically converts values to upper-case when finding matches to internal variables in the simulation.

        Note also that the arguments passed in here can be either strings or bytes, as this wrapper handles conversion
        as needed.

        :param variable_name: The name of the variable to retrieve, e.g. "Site Outdoor Air DryBulb Temperature", or
                              "Fan Air Mass Flow Rate"
        :param variable_key: The instance of the variable to retrieve, e.g. "Environment", or "Main System Fan"
        :return: An integer ID for this output variable, or -1 if one could not be found.
        """
        if isinstance(variable_name, str):
            variable_name = variable_name.encode('utf-8')
        if isinstance(variable_key, str):
            variable_key = variable_key.encode('utf-8')
        return self.api.getVariableHandle(variable_name, variable_key)

    def get_meter_handle(self, meter_name: Union[str, bytes]) -> int:
        """
        Get a handle to a meter in a running simulation.

        The meter name passed into this function do not need to be a particular case, as the EnergyPlus API
        automatically converts values to upper-case when finding matches to internal variables in the simulation.

        Note also that the meter name passed in here can be either strings or bytes, as this wrapper handles conversion
        as needed.

        :param meter_name: The name of the variable to retrieve, e.g. "Electricity:Facility", or "Fans:Electricity"
        :return: An integer ID for this meter, or -1 if one could not be found.
        """
        meter_name = meter_name.upper()
        if isinstance(meter_name, str):
            meter_name = meter_name.encode('utf-8')
        return self.api.getMeterHandle(meter_name)

    def get_actuator_handle(
            self,
            actuator_key: Union[str, bytes],
            component_type: Union[str, bytes],
            control_type: Union[str, bytes]
    ) -> int:
        """
        Get a handle to an available actuator in a running simulation.

        The arguments passed into this function do not need to be a particular case, as the EnergyPlus API
        automatically converts values to upper-case when finding matches to internal variables in the simulation.

        Note also that the arguments passed in here can be either strings or bytes, as this wrapper handles conversion
        as needed.

        :param actuator_key: The instance of the variable to retrieve, e.g. "Environment"
        :param component_type: The actuator category, e.g. "Weather Data"
        :param control_type: The name of the actuator to retrieve, e.g. "Outdoor Dew Point"
        :return: An integer ID for this output variable, or -1 if one could not be found.
        """
        if isinstance(actuator_key, str):
            actuator_key = actuator_key.encode('utf-8')
        if isinstance(component_type, str):
            component_type = component_type.encode('utf-8')
        if isinstance(control_type, str):
            control_type = control_type.encode('utf-8')
        return self.api.getActuatorHandle(actuator_key, component_type, control_type)

    def get_variable_value(self, variable_handle: int) -> float:
        """
        Get the current value of a variable in a running simulation.  The `get_variable_handle` function is first used
        to get a handle to the variable by name.  Then once the handle is retrieved, it is passed into this function to
        then get the value of the variable.

        :param variable_handle: An integer returned from the `get_variable_handle` function.
        :return: Floating point representation of the current variable value
        """
        return self.api.getVariableValue(variable_handle)

    def get_meter_value(self, meter_handle: int) -> float:
        """
        Get the current value of a meter in a running simulation.  The `get_meter_handle` function is first used
        to get a handle to the meter by name.  Then once the handle is retrieved, it is passed into this function to
        then get the value of the meter.

        Note this function is not completed yet.  It currently gives an instant reading of the meter, not an aggregate
        value throughout the simulation.  Use caution

        :param meter_handle: An integer returned from the `get_meter_handle` function.
        :return: Floating point representation of the current meter value
        """
        return self.api.getMeterValue(meter_handle)

    def set_actuator_value(self, actuator_handle: int, actuator_value: RealEP) -> None:
        """
        Sets the value of an actuator in a running simulation.  The `get_actuator_handle` function is first used
        to get a handle to the actuator by name.  Then once the handle is retrieved, it is passed into this function,
        along with the value to assign, to then set the value of the actuator.

        :param actuator_handle: An integer returned from the `get_actuator_handle` function.
        :param actuator_value: The value to assign to the actuator
        :return: Nothing
        """
        self.api.setActuatorValue(actuator_handle, actuator_value)

    def reset_actuator(self, actuator_handle: int) -> None:
        """
        Resets the actuator internally to EnergyPlus.  This allows subsequent calculations to be used for the actuator
        instead of the externally set actuator value.
        // TODO: Add a call to this in the test

        :param actuator_handle: An integer returned from the `get_actuator_handle` function.
        :return: Nothing
        """
        self.api.resetActuator(actuator_handle)

    def get_internal_variable_handle(self, variable_type: Union[str, bytes], variable_key: Union[str, bytes]) -> int:
        """
        Get a handle to an internal variable in a running simulation.

        The arguments passed into this function do not need to be a particular case, as the EnergyPlus API
        automatically converts values to upper-case when finding matches to internal variables in the simulation.

        Note also that the arguments passed in here can be either strings or bytes, as this wrapper handles conversion
        as needed.

        :param variable_type: The name of the variable to retrieve, e.g. "Zone Air Volume", or "Zone Floor Area"
        :param variable_key: The instance of the variable to retrieve, e.g. "Zone 1"
        :return: An integer ID for this output variable, or zero if one could not be found.
        """
        if isinstance(variable_type, str):
            variable_type = variable_type.encode('utf-8')
        if isinstance(variable_key, str):
            variable_key = variable_key.encode('utf-8')
        return self.api.getInternalVariableHandle(variable_type, variable_key)

    def get_internal_variable_value(self, variable_handle: int) -> float:
        """
        Get the value of an internal variable in a running simulation.  The `get_internal_variable_handle` function is
        first used to get a handle to the variable by name.  Then once the handle is retrieved, it is passed into this
        function to then get the value of the variable.

        :param variable_handle: An integer returned from the `get_internal_variable_handle` function.
        :return: Floating point representation of the internal variable value
        """
        return self.api.getInternalVariableValue(variable_handle)

    def get_global_handle(self, var_name: Union[str, bytes]) -> int:
        """
        Get a handle to a global variable in a running simulation.  This is only used for Python Plugin applications!

        Global variables are used as a way to share data between running Python Plugins.  First a global variable must
        be declared in the input file using the PythonPlugin:GlobalVariables object.  Once a name has been declared, it
        can be accessed in the Plugin by getting a handle to the variable using this get_global_handle function, then
        using the get_global_value and set_global_value functions as needed.  Note all global variables are
        floating point values.

        The arguments passed into this function do not need to be a particular case, as the EnergyPlus API
        automatically converts values to upper-case when finding matches to internal variables in the simulation.

        Note also that the arguments passed in here can be either strings or bytes, as this wrapper handles conversion
        as needed.

        :param var_name: The name of the global variable to retrieve, this name must be listed in the IDF object:
                         `PythonPlugin:GlobalVariables`
        :return: An integer ID for this global variable, or zero if one could not be found.
        """
        if not self.running_as_python_plugin:
            raise EnergyPlusException("get_global_handle is only available as part of a Python Plugin workflow")
        if isinstance(var_name, str):
            var_name = var_name.encode('utf-8')
        return self.api.getPluginGlobalVariableHandle(var_name)

    def get_global_value(self, handle: int) -> float:
        """
        Get the current value of a plugin global variable in a running simulation.  This is only used for Python Plugin
        applications!

        Global variables are used as a way to share data between running Python Plugins.  First a global variable must
        be declared in the input file using the PythonPlugin:GlobalVariables object.  Once a name has been declared, it
        can be accessed in the Plugin by getting a handle to the variable using the get_global_handle function, then
        using this get_global_value and the set_global_value functions as needed.  Note all global variables are
        floating point values.

        The arguments passed into this function do not need to be a particular case, as the EnergyPlus API
        automatically converts values to upper-case when finding matches to internal variables in the simulation.

        Note also that the arguments passed in here can be either strings or bytes, as this wrapper handles conversion
        as needed.

        :param handle: An integer returned from the `get_global_handle` function.
        :return: Floating point representation of the global variable value
        """
        if not self.running_as_python_plugin:
            raise EnergyPlusException("get_global_value is only available as part of a Python Plugin workflow")
        return self.api.getPluginGlobalVariableValue(handle)

    def set_global_value(self, handle: int, value: float) -> None:
        """
        Set the current value of a plugin global variable in a running simulation.  This is only used for Python Plugin
        applications!

        Global variables are used as a way to share data between running Python Plugins.  First a global variable must
        be declared in the input file using the PythonPlugin:GlobalVariables object.  Once a name has been declared, it
        can be accessed in the Plugin by getting a handle to the variable using the get_global_handle function, then
        using the get_global_value and this set_global_value functions as needed.  Note all global variables are
        floating point values.

        The arguments passed into this function do not need to be a particular case, as the EnergyPlus API
        automatically converts values to upper-case when finding matches to internal variables in the simulation.

        Note also that the arguments passed in here can be either strings or bytes, as this wrapper handles conversion
        as needed.

        :param handle: An integer returned from the `get_global_handle` function.
        :param value: Floating point value to assign to the global variable
        """
        if not self.running_as_python_plugin:
            raise EnergyPlusException("set_global_handle is only available as part of a Python Plugin workflow")
        self.api.setPluginGlobalVariableValue(handle, value)

    def get_trend_handle(self, trend_var_name: Union[str, bytes]) -> int:
        """

        :param trend_var_name:
        :return:
        """
        if not self.running_as_python_plugin:
            raise EnergyPlusException("get_trend_handle is only available as part of a Python Plugin workflow")
        if isinstance(trend_var_name, str):
            trend_var_name = trend_var_name.encode('utf-8')
        return self.api.getPluginTrendVariableHandle(trend_var_name)

    def get_trend_value(self, trend_handle: int, time_index: int) -> RealEP:
        """

        :param trend_handle:
        :param time_index:
        :return:
        """
        if not self.running_as_python_plugin:
            raise EnergyPlusException("get_trend_value is only available as part of a Python Plugin workflow")
        return self.api.getPluginTrendVariableValue(trend_handle, time_index)

    def get_trend_average(self, trend_handle: int, count: int) -> RealEP:
        if not self.running_as_python_plugin:
            raise EnergyPlusException("get_trend_average is only available as part of a Python Plugin workflow")
        return self.api.getPluginTrendVariableAverage(trend_handle, count)

    def get_trend_min(self, trend_handle: int, count: int) -> RealEP:
        if not self.running_as_python_plugin:
            raise EnergyPlusException("get_trend_min is only available as part of a Python Plugin workflow")
        return self.api.getPluginTrendVariableMin(trend_handle, count)

    def get_trend_max(self, trend_handle: int, count: int) -> RealEP:
        if not self.running_as_python_plugin:
            raise EnergyPlusException("get_trend_max is only available as part of a Python Plugin workflow")
        return self.api.getPluginTrendVariableMax(trend_handle, count)

    def get_trend_sum(self, trend_handle: int, count: int) -> RealEP:
        if not self.running_as_python_plugin:
            raise EnergyPlusException("get_trend_sum is only available as part of a Python Plugin workflow")
        return self.api.getPluginTrendVariableSum(trend_handle, count)

    def get_trend_direction(self, trend_handle: int, count: int) -> RealEP:
        if not self.running_as_python_plugin:
            raise EnergyPlusException("get_trend_direction is only available as part of a Python Plugin workflow")
        return self.api.getPluginTrendVariableDirection(trend_handle, count)

    def year(self) -> int:
        """
        Get the current year of the simulation.

        :return: An integer year
        """
        return self.api.year()

    def month(self) -> int:
        """
        Get the current month of the simulation (1-12)

        :return: An integer month (1-12)
        """
        return self.api.month()

    def day_of_month(self) -> int:
        """
        Get the current day of month (1-31)

        :return: An integer day of the month (1-31)
        """
        return self.api.dayOfMonth()

    def hour(self) -> int:
        """
        Get the current hour of the simulation (1-24) TODO: VERIFY THIS

        :return: An integer hour of the day (1-24)
        """
        return self.api.hour()

    def current_time(self) -> float:
        """
        Get the current time of day in hours TODO: IM NOT SURE WHAT THIS GETS

        :return: A floating point representation of the current time -- fractional hours?
        """
        return self.api.currentTime()

    def minutes(self) -> int:
        """
        Get the current minutes into the hour TODO: VERIFY THIS

        :return: An integer number of minutes into the current hour (1-60) # TODO VERIFY THIS
        """
        return self.api.minutes()

    def day_of_week(self) -> int:
        """
        Get the current day of the week (0-6) TODO: VERIFY THIS

        :return: An integer day of week (0-6)
        """
        return self.api.dayOfWeek()

    def day_of_year(self) -> int:
        """
        Get the current day of the year (1-366) TODO: VERIFY THIS

        :return: AN integer day of the year (1-366)
        """
        return self.api.dayOfYear()

    def daylight_savings_time_indicator(self) -> bool:
        """
        Get the current daylight savings time indicator as a logical value.  The C API returns an integer where 1 is
        yes and 0 is no, this simply wraps that with a bool conversion.

        :return: A boolean DST indicator for the current time.
        """
        return self.api.daylightSavingsTimeIndicator() == 1

    def holiday_index(self) -> int:
        """
        Gets a flag for the current day holiday type: 0 is no holiday, 1 is holiday type #1, etc.

        :return: An integer indicator for current day holiday type.
        """
        return self.api.holidayIndex()

    def sun_is_up(self) -> bool:
        """
        Gets a flag for whether the sun is currently up.  The C API returns an integer where 1 is yes and 0 is no, this
        simply wraps that with a bool conversion.

        :return: A boolean indicating whether the sun is currently up.
        """
        return self.api.sunIsUp() == 1

    def is_raining(self) -> bool:
        """
        Gets a flag for whether the it is currently raining.  The C API returns an integer where 1 is yes and 0 is no,
        this simply wraps that with a bool conversion.

        :return: A boolean indicating whether it is currently raining.
        """
        return self.api.isRaining() == 1

    def warmup_flag(self) -> bool:
        """
        Gets a flag for whether the warmup flag is currently on, signaling that EnergyPlus is still in the process of
        converging on warmup days.  The C API returns an integer where 1 is yes and 0 is no, this simply wraps that
        with a bool conversion.

        :return: A boolean indicating whether the warmup flag is on.
        """
        return self.api.warmupFlag() == 1

    def system_time_step(self) -> float:
        """
        Gets the current system time step value in EnergyPlus.  The system time step is variable and fluctuates
        during the simulation.

        :return: The current system time step in fractional hours.
        """
        return self.api.systemTimeStep()

    def current_environment_num(self) -> int:
        """
        Gets the current environment index.  EnergyPlus environments are design days, run periods, etc.  This function
        is only expected to be useful in very specialized applications where you control the environment order
        carefully.

        :return: The current environment number.
        """
        return self.api.currentEnvironmentNum()
