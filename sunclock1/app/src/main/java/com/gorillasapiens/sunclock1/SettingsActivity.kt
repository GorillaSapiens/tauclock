package com.gorillasapiens.sunclock1

import android.location.LocationManager
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.view.MenuItem
import android.view.View
import android.view.ViewGroup
import android.widget.*
import androidx.appcompat.app.AppCompatActivity
import androidx.core.widget.doOnTextChanged
import androidx.preference.*
import java.util.*

class SettingsActivity : AppCompatActivity() {

    val colors: IntArray = intArrayOf(
        0xFFFFFF00.toInt(), // yellow
        0xFFFFA500.toInt(), // orange
        0xFF4CABF4.toInt(), // lt blue
        0xFF0000FF.toInt(), // blue
        0xFF000080.toInt() // dk blue
    )

    fun colorchange(parent: ViewGroup, light: Int, dark: Int, f: Boolean) : Boolean {
        var second = f;
        for (i in 0 until parent.getChildCount()) {
            val child: View = parent.getChildAt(i)
            if (child is ViewGroup || child is LinearLayout) {
                second = colorchange(child as ViewGroup, light, dark, second) || second
                // DO SOMETHING WITH VIEWGROUP, AFTER CHILDREN HAS BEEN LOOPED
            } else {
                if (child != null) {
                    // DO SOMETHING WITH VIEW
                    if (child is SeekBar) {
                        if (second) {
                            // dark
                            child.getProgressDrawable().setTint(dark)
                            child.getThumb().setTint(dark)
                        }
                        else {
                            // light
                            child.getProgressDrawable().setTint(light)
                            child.getThumb().setTint(light)
                        }
                        second = true
                    }
                }
            }
        }
        return second
    }

    fun technicolor(light: Int, dark: Int) {
        colorchange(window.decorView.rootView as ViewGroup,
            colors[light], colors[4 - dark], false)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.settings_activity)
        if (savedInstanceState == null) {
            supportFragmentManager
                .beginTransaction()
                .replace(R.id.settings, SettingsFragment())
                .commit()
        }
        supportActionBar?.setDisplayHomeAsUpEnabled(true)

        Handler(Looper.getMainLooper() ).postDelayed({
            val sharedPreferences = PreferenceManager.getDefaultSharedPreferences(this /* Activity context */)
            val light = sharedPreferences.getInt("light", 0)
            val dark = sharedPreferences.getInt("dark", 0)
            technicolor(light, dark)
        },150)

    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        when (item.getItemId()) {
            android.R.id.home -> {
                super.onBackPressed()
                return true
            }
        }
        return super.onOptionsItemSelected(item)
    }

    class SettingsFragment : PreferenceFragmentCompat() {

        override fun onCreatePreferences(savedInstanceState: Bundle?, rootKey: String?) {
            setPreferencesFromResource(R.xml.root_preferences, rootKey)

            var manualLocation = findPreference<EditTextPreference>("manual_location")
            if (manualLocation != null) {
                manualLocation.setOnBindEditTextListener { editText ->
                    val parent = editText.parent as ViewGroup
                    if (parent.getChildCount() == 2) {
                        val choices = AutoCompleteTextView(editText.context)
                        val adapter = ArrayAdapter<String>(editText.context,android.R.layout.simple_dropdown_item_1line, resources.getStringArray(R.array.cities))
                        choices.setAdapter(adapter)
                        parent.addView(choices, 1, editText.layoutParams)
                        choices.text = editText.text
                        editText.visibility = View.INVISIBLE
                        editText.height = 0
                        choices.requestFocus()
                        choices.doOnTextChanged { text, start, before, count ->
                            editText.text = choices.text
                        }
                    }
                }
            }

            var locationProviders = findPreference<ListPreference>("provider")
            if (locationProviders != null) {
                var mLocationManager = requireActivity().getSystemService(LOCATION_SERVICE) as LocationManager?
                val allProviders = mLocationManager!!.getProviders(false)
                allProviders.sort()
                allProviders.add(0, "best")
                allProviders.add("manual")
                val array: Array<String> = allProviders.toTypedArray()
                locationProviders.entries = array
                locationProviders.entryValues = array

                val setting = locationProviders.value as String
                if (setting != "manual") {
                    manualLocation!!.setEnabled(false);
                }
                else {
                    manualLocation!!.setEnabled(true);
                }

                locationProviders.setOnPreferenceChangeListener { preference, newValue ->
                    var manualLocation = findPreference<EditTextPreference>("manual_location")
                    val setting = newValue as String
                    if (setting != "manual") {
                        manualLocation!!.setEnabled(false);
                    }
                    else {
                        manualLocation!!.setEnabled(true);
                    }
                    true
                }

            }

            var multiTimezone = findPreference<ListPreference>("manual_timezone")
            if (multiTimezone != null) {
                val timezones = TimeZone.getAvailableIDs()
                multiTimezone.entries = timezones
                multiTimezone.entryValues = timezones
            }

            var timezoneProviders = findPreference<ListPreference>("timezone")
            if (timezoneProviders != null) {

                val setting = timezoneProviders.value as String
                if (setting != "manual") {
                    multiTimezone!!.setEnabled(false);
                }
                else {
                    multiTimezone!!.setEnabled(true);
                }

                timezoneProviders.setOnPreferenceChangeListener { preference, newValue ->
                    var multiTimezone = findPreference<ListPreference>("manual_timezone")
                    val setting = newValue as String
                    if (setting != "manual") {
                        multiTimezone!!.setEnabled(false);
                    }
                    else {
                        multiTimezone!!.setEnabled(true);
                    }
                    true
                }

            }

            var manualOffset = findPreference<EditTextPreference>("manual_offset")
            var offsetProviders = findPreference<ListPreference>("offset")
            if (offsetProviders != null) {

                val setting = offsetProviders.value as String
                if (setting != "manual") {
                    manualOffset!!.setEnabled(false);
                }
                else {
                    manualOffset!!.setEnabled(true);
                }

                offsetProviders.setOnPreferenceChangeListener { preference, newValue ->
                    var manualOffset = findPreference<EditTextPreference>("manual_offset")
                    val setting = newValue as String
                    if (setting != "manual") {
                        manualOffset!!.setEnabled(false);
                    }
                    else {
                        manualOffset!!.setEnabled(true);
                    }
                    true
                }
            }

            var light = findPreference<SeekBarPreference>("light")
            if (light != null) {
                val light_array = getResources().getStringArray(R.array.light_values);
                light.setSummary(light_array[light.value])
                light.setOnPreferenceChangeListener { preference, newValue ->
                    if (newValue is Int) {
                        val light_array = getResources().getStringArray(R.array.light_values);
                        preference.setSummary(light_array[newValue])
                        val other = findPreference<SeekBarPreference>("dark")
                        if (other != null) {
                            val oarray = getResources().getStringArray(R.array.dark_values);
                            while ((other.value + newValue) > 3) {
                                val newOther = other.value - 1;
                                other.value = newOther
                                other.setSummary(oarray[newOther])
                            }
                            val activity = context as SettingsActivity
                            activity.technicolor(newValue, other.value)
                        }
                    }
                    true
                }
            }

            var dark = findPreference<SeekBarPreference>("dark")
            if (dark != null) {
                val dark_array = getResources().getStringArray(R.array.dark_values);
                dark.setSummary(dark_array[dark.value])
                dark.setOnPreferenceChangeListener { preference, newValue ->
                    if (newValue is Int) {
                        val dark_array = getResources().getStringArray(R.array.dark_values);
                        preference.setSummary(dark_array[newValue])
                        val other = findPreference<SeekBarPreference>("light")
                        if (other != null) {
                            val oarray = getResources().getStringArray(R.array.light_values);
                            while (((other.value) + newValue) > 3) {
                                val newOther = other.value - 1;
                                other.value = newOther
                                other.setSummary(oarray[newOther])
                            }
                            val activity = context as SettingsActivity
                            activity.technicolor(other.value, newValue)
                        }
                    }
                    true
                }
            }
        }
    }
}
