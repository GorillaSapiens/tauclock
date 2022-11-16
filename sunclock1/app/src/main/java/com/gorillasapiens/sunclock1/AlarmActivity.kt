package com.gorillasapiens.sunclock1

import android.content.DialogInterface
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Bundle
import android.provider.AlarmClock
import android.text.InputType
import android.view.View
import android.widget.*
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.children
import java.util.*
import kotlin.random.Random

class AlarmActivity : AppCompatActivity() {
    val fieldnames = AlarmStorage.fields
    val categorynames = arrayOf(
        "SOLAR", "CIVIL", "NAUTICAL", "ASTRONOMICAL",
        "LUNAR",                   // moon and planets from here on
        "MERCURY", "VENUS", "MARS", "JUPITER", "SATURN",
        "ARIES")
    val typenames = arrayOf("RISE","TRANSIT","SET")
    var alarmStorage : AlarmStorage? = null
    var mObserver : String = "0.0000,0.0000"

    private fun addEditDelete(title: String, message: String, entry: Int) {
        val table = TableLayout(this)

        var entry = -1

        if (!title.contains("Add")) {
            val hashtagindex = title.indexOf('#')
            val tmp = title.substring(hashtagindex + 1)
            entry = tmp.toInt()
        }

        val values = if (entry != -1) {
            alarmStorage!!.getSet(entry)
        }
        else {
            fieldnames
        }

        var i = 0
        for (field in fieldnames) {
            val tableRow = TableRow(this)
            val tableTextView = TextView(this)
            tableTextView.text = field
            tableTextView.setPadding(32,0,32,0)
            var tableEdit : View =
                if (field == "category") {
                    var tableEditSpinner = Spinner(this)
                    val arrayAdapter = ArrayAdapter(this, android.R.layout.simple_spinner_item, categorynames)
                    tableEditSpinner.adapter = arrayAdapter
                    try {
                        tableEditSpinner.setSelection(values[i]!!.toInt())
                    }
                    catch (e: Exception) {
                        // ignore it
                    }
                    tableEditSpinner
                }
                else if (field == "type") {
                    var tableEditSpinner = Spinner(this)
                    val arrayAdapter = ArrayAdapter(this, android.R.layout.simple_spinner_item, typenames)
                    tableEditSpinner.adapter = arrayAdapter
                    try {
                        tableEditSpinner.setSelection(values[i]!!.toInt())
                    }
                    catch (e: Exception) {
                        // ignore it
                    }
                    tableEditSpinner
                }
                else if (field == "label") {
                    val tableTextView = TextView(this)
                    if (title.contains("Add") || values[i]!!.length == 0) {
                        tableTextView.text = String.format("ταμ-%08x", Random.nextInt())
                    }
                    else {
                        tableTextView.setText(values[i])
                    }
                    tableTextView
                }
                else {
                    val tableEditText = EditText(this)
                    tableEditText.setText(values[i])

                    // special case
                    if (title.contains("Add")) {
                        if (field == "observer") {
                            tableEditText.setText(mObserver)
                        }
                        else if (field == "delay") {
                            tableEditText.setText("0")
                        }
                    }

                    if (field == "delay") {
                        tableEditText.inputType = InputType.TYPE_CLASS_NUMBER or InputType.TYPE_NUMBER_FLAG_SIGNED
                    }

                    tableEditText
                }
            if (title.contains("Delete")) {
                tableEdit.isEnabled = false
            }
            i++
            tableRow.addView(tableTextView)
            tableRow.addView(tableEdit)
            table.addView(tableRow)
        }

        val alertDialogBuilder: AlertDialog.Builder = AlertDialog.Builder(this)
        alertDialogBuilder.setView(table)
        alertDialogBuilder.setTitle(title)
        alertDialogBuilder.setMessage(message)
        alertDialogBuilder.setPositiveButton("Ok",
            DialogInterface.OnClickListener { arg0, arg1 ->
                var entry = -1

                if (!title.contains("Add")) {
                    val hashtagindex = title.indexOf('#')
                    val tmp = title.substring(hashtagindex + 1)
                    entry = tmp.toInt()
                }

                if (title.contains("Del")) {
                    alarmStorage!!.deleteSet(entry)
                    ponderDelete(values[0])
                }
                else {
                    var values : Array<String?> = emptyArray()

                    var i = 0
                    for (field in fieldnames) {
                        val row = table.getChildAt(i) as TableRow
                        if (field == "category" || field == "type") {
                            val spinner = row.getChildAt(1) as Spinner
                            values += spinner.selectedItemPosition.toString()
                        }
                        else if (field == "label") {
                            val textview = row.getChildAt(1) as TextView
                            values += textview.text.toString()
                        }
                        else {
                            val editable = row.getChildAt(1) as EditText
                            values += editable.text.toString()
                        }
                        i++
                    }
                    alarmStorage!!.putSet(entry, values)

                    if (title.contains("Add")) {
                        ponderAdd(values)
                    }
                    else {
                        ponderEdit(values)
                    }
                }
                repopulate()
            })
        alertDialogBuilder.setNegativeButton("Cancel",
            DialogInterface.OnClickListener { arg0, arg1 ->
                // do something
            })
        alertDialogBuilder.setCancelable(false)
        alertDialogBuilder.show()
    }

    private fun ponderAdd(values: Array<String?>) {
        var wakeup_clock = System.currentTimeMillis() / 1000 + 24*60*60 // 24 hours from now
        val alarmStorage = AlarmStorage(this)
        val categorynames = arrayOf(
            "SOLAR", "CIVIL", "NAUTICAL", "ASTRONOMICAL",
            "LUNAR",                   // moon and planets from here on
            "MERCURY", "VENUS", "MARS", "JUPITER", "SATURN",
            "ARIES")
        val typenames = arrayOf("RISE","TRANSIT","SET")

        val label = values[0]
        val description = values[1]
        val observer = values[2]
        val category = values[3]
        val type = values[4]
        val delay = values[5]

        val message = String.format("%s\n%s\n%s %s %s %s%s minutes",
            label,
            description,
            observer,
            categorynames[category!!.toInt()], typenames[type!!.toInt()],
            if (delay?.toInt() ?: 0 < 0) { "-" } else { "+" }, delay)

        val latlon = observer?.split(",")
        val lat = latlon?.get(0)!!.toDouble()
        val lon = latlon.get(1).toDouble()

        val pondering_s = doWhenIsIt(lat, lon,
            category!!.toInt(),type!!.toInt() + 2, delay!!.toInt())

        if (pondering_s > 0) {
            val calendar = Calendar.getInstance()
            calendar.add(Calendar.SECOND, pondering_s)

            val intent = Intent(AlarmClock.ACTION_SET_ALARM)
            intent.putExtra(AlarmClock.EXTRA_HOUR, calendar.get(Calendar.HOUR_OF_DAY)) // Integer
            intent.putExtra(AlarmClock.EXTRA_MINUTES, calendar.get(Calendar.MINUTE)) // Integer
            //intent.putExtra(AlarmClock.EXTRA_IS_PM, calendar.get(Calendar.AM_PM) == 1) // Boolean
            intent.putExtra(AlarmClock.EXTRA_MESSAGE, message)
            intent.putExtra(AlarmClock.EXTRA_SKIP_UI, true)
            this.startActivity(intent)
        }
    }

    private fun ponderEdit(values: Array<String?>) {
        ponderAdd(values)
    }

    private fun ponderDelete(label: String?) {
        val intent = Intent(AlarmClock.ACTION_DISMISS_ALARM)
        intent.putExtra(AlarmClock.EXTRA_ALARM_SEARCH_MODE, AlarmClock.ALARM_SEARCH_MODE_LABEL)
        intent.putExtra(AlarmClock.EXTRA_MESSAGE, label)
        intent.putExtra(AlarmClock.EXTRA_SKIP_UI, true)
        this.startActivity(intent)
    }

    private fun repopulate() {
        val count = alarmStorage!!.getCount()
        val alarmLayout: LinearLayout = findViewById(R.id.alarmLayout)
        alarmLayout.removeAllViews()

        for (i in 0..(count-1)) {
            val values = alarmStorage!!.getSet(i)

            val tv = TextView(this)
            tv.text = values[1]
            if (tv.text.length == 0) {
                tv.text = "<none>"
            }
            tv.setTextColor(0xFFFFFFFF.toInt())
            tv.setTextSize(32.0f)

            tv.setOnClickListener { v ->
                val alarmLayout: LinearLayout = findViewById(R.id.alarmLayout)
                for (child in alarmLayout.children) {
                    val tv = child as TextView
                    tv.setTextColor(0xffffffff.toInt())
                }
                val tv = v as TextView
                tv.setTextColor(0xffff00ff.toInt())
            }

            alarmLayout.addView(tv)
        }

        val alarmHowMany: TextView = findViewById(R.id.alarmHowMany)
        alarmHowMany.text = count.toString() + " shown"
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_alarm)

        val manager = this.packageManager
        val info = manager.getPackageInfo(this.packageName, PackageManager.GET_ACTIVITIES)
        actionBar?.title = "ταμ clock v" + info.versionName
        supportActionBar?.title = "ταμ clock v" + info.versionName

        val b = intent.extras
        var value : String? = "0.0000,0.0000"
        if (b != null) value = b.getString("observer")
        if (value != null) {
            mObserver = value
        }

        val backButton: Button = findViewById(R.id.backButton)
        backButton.setOnClickListener { v ->
            finish()
        }

        alarmStorage = AlarmStorage(this)
        val alarmLayout: LinearLayout = findViewById(R.id.alarmLayout)

        val addButton: Button = findViewById(R.id.addButton)
        addButton.setOnClickListener { v ->
            addEditDelete("Add", "Add new alarm", -1)
        }

        val editButton: Button = findViewById(R.id.editButton)
        editButton.setOnClickListener { v ->
//            val entry = findEntry(alarmLayout)
//            if (entry != -1) {
//                addEditDelete("Edit #" + entry, "Edit existing alarm", entry)
            AlertDialog.Builder(this)
                .setTitle("Unimplemented")
                .setMessage("'Edit' is unimplemented.  As a workaround, please manually delete this alarm and add a new one'.") // Specifying a listener allows you to take an action before dismissing the dialog.
                .setPositiveButton(
                    android.R.string.yes
                ) { dialog, which ->

                }
                .setIcon(android.R.drawable.ic_dialog_alert)
                .show()
        }

        val deleteButton: Button = findViewById(R.id.deleteButton)
        deleteButton.setOnClickListener { v ->
            val entry = findEntry(alarmLayout)
            if (entry != -1) {
                addEditDelete("Delete #" + entry, "Delete this alarm.\nAre you sure?", entry)
            }
        }

        repopulate()
    }

    private fun findEntry(alarmLayout: LinearLayout): Int {
        var ret = 0.toInt()
        for (child in alarmLayout.children) {
            val tv= child as TextView
            if (tv.getCurrentTextColor() == 0xffff00ff.toInt()) {
                return ret
            }
            ret++
        }
        return -1
    }

    private external fun doWhenIsIt(lat:Double, lon:Double, category:Int, type:Int, delayMinutes:Int) : Int
}

