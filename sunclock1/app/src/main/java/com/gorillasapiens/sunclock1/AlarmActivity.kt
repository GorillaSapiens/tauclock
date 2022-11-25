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

                    // special case
                    if (title.contains("Add")) {
                        if (field == "observer") {
                            tableEditText.setText(mObserver)
                        }
                        else if (field == "delay") {
                            tableEditText.setText("")
                            tableEditText.hint = "0"
                        }
                        else {
                            tableEditText.setText("")
                            tableEditText.hint = values[i]
                        }
                    }
                    else {
                        tableEditText.setText(values[i])
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
                    val hourminute = alarmStorage!!.getLedger(values[0])
                    alarmStorage!!.deleteSet(entry)
                    alarmStorage!!.deleteLedger(values[0])
                    alarmStorage!!.ponderDelete(values[0], hourminute[0].toInt(), hourminute[1].toInt())
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
                        else if (field == "delay") {
                            val editable = row.getChildAt(1) as EditText
                            var value = 0
                            try {
                                value = editable.text.toString().toInt()
                            }
                            catch (e: Exception) {
                                // do nothing
                            }

                            values += value.toString()
                        }
                        else {
                            val editable = row.getChildAt(1) as EditText
                            values += editable.text.toString()
                        }
                        i++
                    }
                    alarmStorage!!.putSet(entry, values)

                    if (title.contains("Add")) {
                        alarmStorage!!.ponderAdd(values)
                    }
                    else {
                        alarmStorage!!.ponderEdit(values)
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

    private fun repopulate() {
        val count = alarmStorage!!.getCount()
        val alarmLayout: LinearLayout = findViewById(R.id.alarmLayout)
        alarmLayout.removeAllViews()

        for (i in 0..(count-1)) {
            val values = alarmStorage!!.getSet(i)

            val tv = TextView(this)
            tv.text = values[1] // bad deep voodoo
            if (tv.text.length == 0) {
                val observer = values[2]
                val category = values[3]
                val type = values[4]
                val delay = values[5]

                val message = String.format("%s %s %s minutes at %s",
                    categorynames[category!!.toInt()], typenames[type!!.toInt()],
                    alarmStorage!!.delayString(delay),
                    observer)

                tv.text = message
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
            val entry = findEntry(alarmLayout)
            if (entry != -1) {
                addEditDelete("Edit #" + entry, "Edit existing alarm", entry)
            }
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
}

