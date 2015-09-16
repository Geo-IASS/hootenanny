package hoot.services.review;

import hoot.services.UnitTest;
import hoot.services.utils.XmlDocumentBuilder;

import java.io.File;

import org.apache.commons.io.FileUtils;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.experimental.categories.Category;
import org.w3c.dom.Document;

public class ReviewItemsUpdaterTest
{
  private static Document changesetDoc;
	
	@BeforeClass
  public static void beforeClass() throws Exception
  {
		changesetDoc = 
			XmlDocumentBuilder.parse(
        FileUtils.readFileToString(
          new File(
            Thread.currentThread().getContextClassLoader().getResource(
              "hoot/services/review/ReviewItemsUpdater-testUpdateReviewItems.osm")
            .getPath())));
  }
	
	@Ignore
	@Test
	@Category(UnitTest.class)
	public void testCreateReviewRecordsFromCreateChangeset() throws Exception
	{
		
	}
	
	@Ignore
	@Test
	@Category(UnitTest.class)
	public void testCreateReviewRecordsFromModifyChangeset() throws Exception
	{
		
	}
	
	@Ignore
	@Test
	@Category(UnitTest.class)
	public void testGetDeleteUniqueIdsFromChangeset() throws Exception
	{
		//ReviewItemsRetriever reviewItemsUpdater = Mockito.spy(new ReviewItemsRetriever());
		//reviewItemsUpdater.setMapId(1);
		//reviewItemsUpdater.setUserId(1);
		//reviewItemsUpdater.ge
	}
	
	/*
	@Ignore
	@Test
	@Category(IntegrationTest.class)
	public void testUpdateReviewItems() throws Exception
	{
		
	}*/
}
