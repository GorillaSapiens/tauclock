package com.gorillasapiens.sunclock1

import android.location.LocationManager
import android.os.Bundle
import android.view.MenuItem
import android.view.View
import android.view.ViewGroup
import android.widget.*
import androidx.appcompat.app.AppCompatActivity
import androidx.core.widget.doOnTextChanged
import androidx.preference.EditTextPreference
import androidx.preference.ListPreference
import androidx.preference.PreferenceFragmentCompat
import java.util.*


class SettingsActivity : AppCompatActivity() {

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
        }
    }
}
